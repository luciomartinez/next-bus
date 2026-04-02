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
  - the closer the bus is, the brighter the display
  - the farther away the bus is, the dimmer the display
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

Common [`arduino-cli`](https://arduino.github.io/arduino-cli) commands used for this project:

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

## Customization

The main project configuration lives in `next_bus.ino`.

### Change the number of displays

This project currently creates one `TM1637Display` object per physical display:

```cpp
TM1637Display display1(CLK1, DIO1);
TM1637Display display2(CLK2, DIO2);
TM1637Display display3(CLK3, DIO3);
TM1637Display* displays[] = {&display1, &display2, &display3};
```

If you want fewer or more displays:

1. Add or remove the `CLK` / `DIO` pin definitions.
2. Add or remove the `TM1637Display displayX(...)` objects.
3. Add or remove entries in the `displays[]` array.
4. Make sure `displayTargets` has the same number of entries as `displays[]`.

### Change the wiring

Each display is wired by its `CLK` and `DIO` pin pair:

```cpp
#define CLK1 5
#define DIO1 4
```

To move a display to different pins, just change those values and upload again.

### Change which bus each display shows

Each display target is configured in `displayTargets`:

```cpp
const TrafiklabApi::DisplayTarget displayTargets[DISPLAY_COUNT] = {
  {TrafiklabApi::tollareTorgAreaId, "414", "Slussen", "Tollare torg 414 -> Slussen"},
  {TrafiklabApi::hedenstromsVagAreaId, "442", "Slussen", "Hedenstroms vag 442 -> Slussen"},
  {TrafiklabApi::tollareTorgAreaId, "442X", "Glasbruksgatan", "Tollare torg 442X -> Glasbruksgatan"}
};
```

Each entry is:

```cpp
{areaId, line, direction, label}
```

- `areaId`: Trafiklab stop-group / area id
- `line`: bus line, for example `"414"` or `"442X"`
- `direction`: the destination / direction name used by Trafiklab
- `label`: only used for serial debug output

### Change the stop / area id

The project currently includes two named area ids in `TrafiklabApi.h`:

- `TrafiklabApi::tollareTorgAreaId`
- `TrafiklabApi::hedenstromsVagAreaId`

You can either:

- reuse those constants
- add new constants in `TrafiklabApi.h` and `TrafiklabApi.cpp`
- or put the raw area id string directly into `displayTargets`

Example:

```cpp
{"740012345", "471", "Centralen", "My stop 471 -> Centralen"}
```

You can find area ids through Trafiklab's stop lookup and developer portal.

## Remote Control

The ESP32 advertises itself on the local network as:

- `http://next-bus.local`

Example control URLs:

- `http://next-bus.local/toggle`
- `http://next-bus.local/status`

This works well with Apple Shortcuts, but any client that can make local HTTP requests can use these endpoints.

If `.local` resolution is slow or unreliable on a client, the board IP can be used instead.

## TODO

- Keep countdowns moving during temporary API or network failures by caching the last successful per-display departure state and locally decrementing displayed minutes until fresh Trafiklab data arrives.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
