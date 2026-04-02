# Next Bus

> See the next bus countdown at home and control it remotely.

This project uses an ESP32 and three TM1637 displays to show:

- the next `414` from _Tollare torg_ to _Slussen_
- the next `442` from _Hedenströms väg_ to _Slussen_
- the next `442X` from _Tollare torg_ to _Glasbruksgatan_

Displays stay blank when no matching departure is within 59 minutes.

## Features

- Trafiklab realtime departures
- dynamic display brightness based on countdown
- local HTTP remote control:
  - `/on`
  - `/off`
  - `/toggle`
  - `/status`

## Hardware

- ESP32
- 3 x TM1637 4-digit displays

Display wiring:

- display 1: `CLK 5`, `DIO 4`
- display 2: `CLK 18`, `DIO 19`
- display 3: `CLK 21`, `DIO 22`

## Setup

1. Copy `secrets.example.h` to `secrets.h`.
2. Fill in your Wi-Fi credentials and Trafiklab API key.
3. Compile and upload the sketch to your ESP32.

Helpful Trafiklab links:

- Getting started and API keys: [Using Trafiklab.se](https://www.trafiklab.se/docs/getting-started/using-trafiklab/)
- Trafiklab developer portal: [developer.trafiklab.se](https://developer.trafiklab.se/)
- Timetables API docs: [Trafiklab Timetables](https://www.trafiklab.se/api/our-apis/trafiklab-realtime-apis/timetables/)

Current board target used during development:

- FQBN: `esp32:esp32:alksesp32`

## Development

Common `arduino-cli` commands used for this project:

```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:alksesp32 .

# Upload
arduino-cli upload -p /dev/cu.usbserial-0001 --fqbn esp32:esp32:alksesp32 .

# Monitor serial output
arduino-cli monitor -p /dev/cu.usbserial-0001 -c baudrate=115200

# List connected boards
arduino-cli board list
```

## Remote Control

The ESP32 advertises itself on the local network as:

- `http://next-bus.local`

Example control URLs:

- `http://next-bus.local/toggle`
- `http://next-bus.local/status`

This works well with Apple Shortcuts, but any client that can make local HTTP requests can use these endpoints.

If `.local` resolution is slow or unreliable on a client, the board IP can be used instead.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
