# next-bus

> See the next bus countdown at home and control it remotely.

This project uses an ESP32 and three TM1637 displays to show:

- the next `414` from Tollare torg to Slussen
- the next `442` from Hedenstroms vag to Slussen
- the next `442X` from Tollare torg to Glasbruksgatan

Displays stay blank when no matching departure is within 59 minutes.

## Hardware

- ESP32
- 3 x TM1637 4-digit displays

Display wiring:

- display 1: `CLK 5`, `DIO 4`
- display 2: `CLK 18`, `DIO 19`
- display 3: `CLK 21`, `DIO 22`

## Features

- Trafiklab realtime departures
- dynamic display brightness based on countdown
- local HTTP control for Shortcuts:
  - `/on`
  - `/off`
  - `/toggle`
  - `/status`

## Setup

1. Copy `secrets.example.h` to `secrets.h`.
2. Fill in your Wi-Fi credentials and Trafiklab API key.
3. Compile and upload the sketch to your ESP32.

Current board target used during development:

- FQBN: `esp32:esp32:alksesp32`

## Shortcuts

The ESP32 advertises itself on the local network as:

- `http://next-bus.local`

Example Shortcuts URLs:

- `http://next-bus.local/toggle`
- `http://next-bus.local/status`

If `.local` resolution is slow or unreliable on a client, the board IP can be used instead.
