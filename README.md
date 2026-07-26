# Picozap

Picozap is a Wi-Fi and Bluetooth enabled, remotely managed keystroke injection
tool built around the **RP2040** microcontroller and
the **CYW43439** wireless chip. It runs on a custom board but is fully compatible
with the **Raspberry Pi Pico W** — add two buttons and an SSD1306 128×32 I²C OLED
to a Pico W and you have a working device.

Unlike traditional USB-only injection tools, Picozap can be driven entirely over
the air. Once powered, it either joins a saved Wi-Fi network or brings up its own
setup portal, then hosts a web interface for uploading, managing and triggering
DuckyScript payloads — no physical interaction with the device required after it
is planted. Payloads can be typed to the target over **USB HID** or wirelessly
over **Bluetooth LE HID**.

## Features

* **Dual HID transport (USB & BLE):** Inject keystrokes over USB or wirelessly
  over Bluetooth LE (HID-over-GATT). The active transport is switchable at
  runtime, with an optional "always on" mode.
* **Wireless payload execution:** Trigger any stored payload remotely from the
  built-in web interface.
* **Captive-portal Wi-Fi setup:** On first boot (or after "forget network") the
  device starts a soft-AP with a DHCP/DNS captive portal so you can scan for and
  join a network from your phone — no hardcoded credentials needed.
* **SD-card payload storage:** Store and manage multiple DuckyScript payloads on
  the on-board SD card (FatFs over SPI).
* **Over-the-air uploads:** Upload new payload files straight to the SD card
  through the web server.
* **Live configuration:** Adjust DuckyScript timing (char/fuzz delays),
  keyboard layout and HID transport remotely; settings persist to the SD card.
* **Keyboard layouts:** US-QWERTY and TR-QWERTY supported.
* **mDNS discovery:** Reach the device by hostname on the local network instead
  of hunting for its IP.
* **On-device GUI:** SSD1306 OLED (via u8g2) with a menu-driven view system for
  status, settings and network state, plus physical control buttons.
* **Power management:** Battery voltage/charge monitoring and dormant deep-sleep
  for low-power standalone use.
* **C SDK based:** Built natively on the Pico C/C++ SDK and FreeRTOS for
  predictable, low-overhead performance.

## Architecture

| Layer | Details |
|---|---|
| MCU | RP2040 (dual-core Cortex-M0+, 264 KB SRAM) |
| SDK | Raspberry Pi Pico SDK 2.3.0 (CMake, C11) |
| RTOS | FreeRTOS SMP |
| Wireless | CYW43439 via `pico_cyw43_arch` |
| Network | lwIP (`NO_SYS=0`, FreeRTOS TCP/IP thread) + lwIP httpd (SSI/CGI/POST) |
| Bluetooth | BTstack (BLE HID over GATT) |
| USB | TinyUSB HID (keyboard emulation) |
| Storage | FatFs over SPI SD card |
| Display | SSD1306 OLED via u8g2 |

The source is organized under `src/` into `app/` (duckyscript engine, hid,
gui, web server, net manager, power manager), `hal/` (board pin mapping and
drivers), `middleware/` (fat_io, config store, input, radio/sleep managers)
and `config/` (SDK, FreeRTOS, lwIP, TinyUSB and BTstack configuration).

## Hardware

Picozap runs on a custom board but is fully **Pico W compatible**. To run it on a
plain Raspberry Pi Pico W you only need to add:

* **2 buttons** — SELECT and DOWN, for navigating the on-device menu.
* **SSD1306 128×32 I²C OLED** — for local status and the menu UI.
* **SD card (SPI)** — for payload and settings storage.

The full custom board additionally provides battery power with MCP73833 charger status
sensing for standalone, off-USB operation. These extras are optional on a Pico W.

Default pin assignments (buttons, OLED, SD card, battery/charger, status LED)
live in `src/hal/hal.h` — adjust them to match your wiring.

*Note: Full hardware schematics for the custom board will be published in a
separate repository.*

## Building

### Prerequisites

* Raspberry Pi Pico SDK **2.3.0** and the matching ARM toolchain (the
  Raspberry Pi Pico VS Code extension installs these under `~/.pico-sdk`).
* `pico-extras`.
* Python 3 (used at build time to generate the web content).
* CMake ≥ 3.13.

### Clone

```sh
git clone --recurse-submodules <repo-url>
cd picozap
```

The FreeRTOS kernel is a git submodule; if you already cloned without
submodules, run `git submodule update --init --recursive`.

### Configure and build

```sh
cmake -B build -S .
cmake --build build
```

The build targets the **Pico W** board (`PICO_BOARD=pico_w`) by default, so no
board configuration is needed for either the custom board or a stock Pico W.

Flash the resulting `build/picozap.uf2` to the board (BOOTSEL mode, or via
`picotool`).

## Disclaimer

This project is developed solely for educational purposes, security research,
and authorized penetration testing. Unauthorized or illegal use of this tool is
strictly prohibited. The developer(s) assume no liability and are not
responsible for any misuse or damage caused by this program. The user assumes
full responsibility for their actions.
