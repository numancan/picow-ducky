#!/usr/bin/env python3
"""Generate the lwIP httpd content tree by injecting C `$EXPORT` markers.

The firmware marks its source-of-truth definitions with a comment placed directly
above the macro that defines them, e.g.

    // $EXPORT=ID,NAME
    #define HID_KEY_LAYOUT_LIST(X) \
        X(HID_KEY_LAYOUT_US_Q, "US-Q", ascii_table_us) ...

    // $EXPORT=NAME,KEYCODE
    #define HID_STRING_TO_SPECIAL_KEY \
        {"CTRL", HID_KEY_CONTROL_LEFT}, {"RCTRL", HID_KEY_CONTROL_RIGHT}, ...

This script scans every `.h` under --src-root for those markers, parses the macro
that follows (either an X-macro list `X(...)` or a brace-init list `{...}`), and
builds a JSON object keyed by the macro name:

    { "HID_KEY_LAYOUT_LIST": [ {"ID": "...", "NAME": "US-Q"}, ... ], ... }

It then copies the whole --content-src tree into --content-out, replacing the
`/*{{EXPORT}}*/{}` placeholder in text files with that object literal so the served web
UI stays in sync with the C definitions. A page that only needs a few of the macros can
scope the placeholder to a comma-separated allowlist, e.g. `/*{{EXPORT:NET_STATUS_LIST}}*/{}`,
so it only embeds those macros instead of the full export set. A malformed marker, or a
scoped placeholder naming a macro that wasn't exported anywhere, is a hard error so the
build fails loudly.
"""
import argparse
import json
import re
import sys
from pathlib import Path

# Marker line: `// $EXPORT=field1,field2,...`
MARKER_RE = re.compile(r"//\s*\$EXPORT\s*=\s*(\S.*?)\s*$")
# `const EXPORTS = /*{{EXPORT}}*/{};` placeholder (kept as a valid empty object literal
# so the source file is still runnable on its own). An optional `:NAME,NAME,...` scopes
# the substitution to just those macro names instead of the full export set.
PLACEHOLDER_RE = re.compile(r"/\*\{\{EXPORT(?::([A-Za-z0-9_,\s]+))?\}\}\*/\s*\{\}")
TEXT_EXTS = {".js", ".html", ".htm", ".css", ".shtml", ".shtm", ".json", ".svg"}


class ExportError(RuntimeError):
    pass


def capture_balanced(text, start, open_c, close_c):
    """text[start] must be open_c. Return (inner_text, index_after_close)."""
    depth = 0
    in_str = False
    esc = False
    i = start
    while i < len(text):
        c = text[i]
        if in_str:
            if esc:
                esc = False
            elif c == "\\":
                esc = True
            elif c == '"':
                in_str = False
        elif c == '"':
            in_str = True
        elif c == open_c:
            depth += 1
        elif c == close_c:
            depth -= 1
            if depth == 0:
                return text[start + 1:i], i + 1
        i += 1
    raise ExportError(f"unbalanced '{open_c}{close_c}'")


def split_top_level(s):
    """Split on top-level commas, respecting quotes and (){}[] nesting."""
    args = []
    depth = 0
    in_str = False
    esc = False
    cur = []
    for c in s:
        if in_str:
            cur.append(c)
            if esc:
                esc = False
            elif c == "\\":
                esc = True
            elif c == '"':
                in_str = False
        elif c == '"':
            in_str = True
            cur.append(c)
        elif c in "([{":
            depth += 1
            cur.append(c)
        elif c in ")]}":
            depth -= 1
            cur.append(c)
        elif c == "," and depth == 0:
            args.append("".join(cur).strip())
            cur = []
        else:
            cur.append(c)
    tail = "".join(cur).strip()
    if tail:
        args.append(tail)
    return args


def parse_value(arg):
    """A quoted string becomes its contents; any other token is kept verbatim."""
    arg = arg.strip()
    if len(arg) >= 2 and arg[0] == '"' and arg[-1] == '"':
        return arg[1:-1]
    return arg


def extract_paren_groups(text, token):
    """Return the inner text of each `token(...)` invocation (e.g. token='X')."""
    groups = []
    i = 0
    while i < len(text):
        idx = text.find(token, i)
        if idx == -1:
            break
        before = text[idx - 1] if idx > 0 else " "
        j = idx + len(token)
        while j < len(text) and text[j] in " \t\r\n":
            j += 1
        if (before.isalnum() or before == "_") or j >= len(text) or text[j] != "(":
            i = idx + len(token)
            continue
        inner, end = capture_balanced(text, j, "(", ")")
        groups.append(inner)
        i = end
    return groups


def extract_brace_groups(text):
    """Return the inner text of each top-level `{...}` group."""
    groups = []
    i = 0
    while i < len(text):
        idx = text.find("{", i)
        if idx == -1:
            break
        inner, end = capture_balanced(text, idx, "{", "}")
        groups.append(inner)
        i = end
    return groups


def collect_macro_body(lines, def_idx):
    """Join the `#define` at def_idx and its backslash-continued lines into one string."""
    parts = []
    i = def_idx
    while i < len(lines):
        stripped = lines[i].rstrip()
        cont = stripped.endswith("\\")
        parts.append(stripped[:-1] if cont else stripped)
        i += 1
        if not cont:
            break
    return " ".join(parts)


def parse_marker(lines, marker_idx, fields, header):
    """Parse the macro following a marker at marker_idx into a list of row dicts."""
    def_idx = None
    for j in range(marker_idx + 1, len(lines)):
        if lines[j].lstrip().startswith("#define"):
            def_idx = j
            break
    if def_idx is None:
        raise ExportError(f"{header}: no #define found after $EXPORT marker")

    body = collect_macro_body(lines, def_idx)
    m = re.match(r"\s*#define\s+(\w+)\s*(\([^)]*\))?", body)
    if not m:
        raise ExportError(f"{header}: could not parse #define after marker")
    name = m.group(1)
    is_func_like = m.group(2) is not None
    remainder = body[m.end():]

    raw_groups = extract_paren_groups(remainder, "X") if is_func_like else extract_brace_groups(remainder)
    if not raw_groups:
        raise ExportError(f"{header}: macro '{name}' produced no rows")

    rows = []
    for group in raw_groups:
        args = split_top_level(group)
        if len(args) < len(fields):
            raise ExportError(
                f"{header}: macro '{name}' row has {len(args)} column(s), "
                f"marker declares {len(fields)} field(s): {{{group.strip()}}}"
            )
        rows.append({field: parse_value(args[k]) for k, field in enumerate(fields)})
    return name, rows


def collect_exports(src_root):
    exports = {}
    for header in sorted(Path(src_root).rglob("*.h")):
        try:
            text = header.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        lines = text.splitlines()
        for idx, line in enumerate(lines):
            mk = MARKER_RE.search(line)
            if not mk:
                continue
            fields = [f.strip().upper() for f in mk.group(1).split(",") if f.strip()]
            if not fields:
                raise ExportError(f"{header}: empty $EXPORT field list")
            name, rows = parse_marker(lines, idx, fields, str(header))
            if name in exports:
                raise ExportError(f"duplicate $EXPORT macro name '{name}' (in {header})")
            exports[name] = rows
    return exports


INT_LITERAL_RE = re.compile(r"^(0[xX][0-9a-fA-F]+|\d+)[uUlL]*$")
# object-like `#define NAME value` (function-like `#define NAME(...)` won't match: `(`
# follows NAME with no whitespace, so `\s+` fails).
DEFINE_RE = re.compile(r"^\s*#define\s+([A-Za-z_]\w*)\s+(.+?)\s*$")


def collect_defines(src_root):
    """Map object-like `#define NAME value` from all headers (value kept as raw text)."""
    defines = {}
    for header in sorted(Path(src_root).rglob("*.h")):
        try:
            text = header.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        for line in text.splitlines():
            m = DEFINE_RE.match(line)
            if not m:
                continue
            value = re.sub(r"/\*.*?\*/", "", m.group(2)).split("//")[0].strip()
            defines[m.group(1)] = value
    return defines


def resolve_number(token, defines, exports, seen=None):
    """Resolve a bare token to an int, or None if it isn't numeric.

    Handles integer literals, object-like #define chains, and `<PREFIX>_COUNT` enum
    counts (== the row count of the matching `<PREFIX>_LIST` export).
    """
    if seen is None:
        seen = set()
    t = token.strip()
    m = INT_LITERAL_RE.match(t)
    if m:
        return int(m.group(1), 0)
    if t in defines and t not in seen:
        seen.add(t)
        return resolve_number(defines[t], defines, exports, seen)
    if t.endswith("_COUNT"):
        list_key = t[: -len("_COUNT")] + "_LIST"
        if list_key in exports:
            return len(exports[list_key])
    return None


def resolve_values(exports, defines):
    """In-place: turn any field value that names a numeric constant into an int.

    Quoted-string fields ("US-Q", "Char Delay") and enum IDs / type tokens (which don't
    resolve) are left untouched.
    """
    for rows in exports.values():
        for row in rows:
            for key, value in row.items():
                if isinstance(value, str):
                    num = resolve_number(value, defines, exports)
                    if num is not None:
                        row[key] = num


def render_placeholder(match, exports, header):
    names_group = match.group(1)
    if names_group is None:
        subset = exports
    else:
        names = [n.strip() for n in names_group.split(",") if n.strip()]
        subset = {}
        for name in names:
            if name not in exports:
                raise ExportError(f"{header}: $EXPORT placeholder requests unknown macro '{name}'")
            subset[name] = exports[name]
    return json.dumps(subset, ensure_ascii=False)


def build_content(content_src, content_out, exports):
    src = Path(content_src)
    out = Path(content_out)
    replaced = 0
    for path in sorted(src.rglob("*")):
        if path.is_dir():
            continue
        dst = out / path.relative_to(src)
        dst.parent.mkdir(parents=True, exist_ok=True)
        data = path.read_bytes()
        if path.suffix.lower() in TEXT_EXTS:
            text = data.decode("utf-8")
            text, n = PLACEHOLDER_RE.subn(lambda m: render_placeholder(m, exports, path), text)
            replaced += n
            dst.write_bytes(text.encode("utf-8"))
        else:
            dst.write_bytes(data)
    return replaced


def main():
    ap = argparse.ArgumentParser(description="Inject C $EXPORT markers into web content")
    ap.add_argument("--content-src", required=True, help="source content directory")
    ap.add_argument("--content-out", required=True, help="generated content output directory")
    ap.add_argument("--src-root", required=True, help="root to scan for .h $EXPORT markers")
    args = ap.parse_args()

    try:
        exports = collect_exports(args.src_root)
        resolve_values(exports, collect_defines(args.src_root))
        replaced = build_content(args.content_src, args.content_out, exports)
    except ExportError as e:
        print(f"gen_web_content: error: {e}", file=sys.stderr)
        return 1

    print(f"gen_web_content: {len(exports)} export list(s); placeholder replaced {replaced}x")
    return 0


if __name__ == "__main__":
    sys.exit(main())
