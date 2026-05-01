# Picow Ducky

Picow Ducky is a Wi-Fi enabled, remote-controlled Keystroke Injection tool (Rubber Ducky) designed for custom hardware utilizing the RP2040 microcontroller and the CYW43439 wireless chip. 

Developed with the Raspberry Pi Pico C/C++ SDK, this project differentiates itself from traditional USB injection tools by allowing full remote management over a local network. Once plugged into a target, it connects to an existing Wi-Fi network and hosts a web server, giving you complete control without physical intervention.

## Features

* **Wireless Payload Execution:** Trigger any payload remotely via the built-in web interface.
* **SD Card Payload Storage:** Store and manage multiple payloads directly on the onboard SD card.
* **Over-the-Air Payload Uploads:** Upload new payload files directly to the device's SD card through the web server.
* **Live Configuration:** Modify device settings remotely on the fly.
* **Web Server Interface:** Hosts an internal HTML/CSS web server upon connecting to a predefined Wi-Fi network (Station Mode).
* **Hardware Integrations:** Fully supports OLED screens for real-time status updates, battery power for standalone operations, and extra physical control buttons.
* **C SDK Based:** Built natively using the Pico C/C++ SDK for optimal performance.

## Hardware Requirements

This software is specifically optimized for custom boards featuring:
* **MCU:** RP2040
* **Wireless Module:** Infineon CYW43439
* **Storage:** SD Card module
* **Display:** OLED screen
* **Power:** Battery support
* **Input:** Extra control buttons

*Note: Specific hardware schematics and details regarding the custom board will be published in a separate repository.*

## Usage

*(To be updated...)*

## Disclaimer

This project is developed solely for educational purposes, security research, and authorized penetration testing. Unauthorized or illegal use of this tool is strictly prohibited. The developer(s) assume no liability and are not responsible for any misuse or damage caused by this program. The user assumes full responsibility for their actions.