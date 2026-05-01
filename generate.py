import os
import re
from pathlib import Path

# Configuration
SRC_DIR = "src"                     # Root source directory
OUT_FILE = "views_combined.c"       # Output filename

# Regex patterns to capture muif_list and fds_data arrays
MUIF_PATTERN = re.compile(r"muif_t\s+muif_list\[\]\s*=\s*\{([\s\S]*?)\};", re.MULTILINE)
FDS_PATTERN = re.compile(r"fds_t\s+fds_data\[\]\s*=\s*([^;]+);", re.MULTILINE)

muif_entries = []
fds_entries = []
included_headers = set()

# Traverse all files recursively
for path in Path(SRC_DIR).rglob("view_*.h"):
    text = path.read_text()

    # Collect muif_list contents
    muif_match = MUIF_PATTERN.search(text)
    if muif_match:
        muif_entries.append(muif_match.group(1).strip())

    # Collect fds_data contents
    fds_match = FDS_PATTERN.search(text)
    if fds_match:
        fds_entries.append(fds_match.group(1).strip())

    # Collect includes for deduplication
    for line in text.splitlines():
        if line.strip().startswith("#include"):
            included_headers.add(line.strip())

# Generate output
output = []
output.append("// Auto-generated file. Do not edit.\n")
output.extend(sorted(included_headers))
output.append("\n")

output.append("muif_t muif_all[] = {\n")
for block in muif_entries:
    output.append(block.strip())
    if not block.strip().endswith(","):
        output.append(",")
    output.append("\n")
output.append("};\n\n")

output.append("fds_t fds_all[] =\n")
for block in fds_entries:
    output.append(block.strip())
    output.append("\n")
output.append(";\n")

# Write to file
Path(OUT_FILE).write_text("".join(output))

print(f"✅ Generated {OUT_FILE} with {len(muif_entries)} view(s).")
