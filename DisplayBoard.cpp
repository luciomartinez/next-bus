#include "DisplayBoard.h"

namespace DisplayBoard {

namespace {

const uint8_t minVisibleBrightness = 1;
const uint8_t maxBrightness = 7;

struct BrightnessBand {
  int maxMinutes;
  uint8_t brightness;
};

const BrightnessBand brightnessBands[] = {
  {5, maxBrightness},
  {9, 6},
  {14, 5},
  {19, 4},
  {29, 3},
  {45, 2},
  {59, minVisibleBrightness},
};

const uint8_t blankSegments[] = {0x00, 0x00, 0x00, 0x00};

uint8_t brightnessForValue(int value, uint8_t defaultBrightness, bool dynamicBrightnessEnabled) {
  if (!dynamicBrightnessEnabled || value < 0) {
    return defaultBrightness;
  }

  // TM1637 brightness is in the range 0..7, where 7 is brightest.
  // Nearer departures render brighter so the most urgent bus stands out first.
  for (const BrightnessBand& band : brightnessBands) {
    if (value <= band.maxMinutes) {
      return band.brightness;
    }
  }

  return defaultBrightness;
}

void clear(TM1637Display& display) {
  display.setSegments(blankSegments);
}

}  // namespace

void initialize(TM1637Display* const* displays, size_t displayCount, uint8_t brightness) {
  for (size_t i = 0; i < displayCount; ++i) {
    displays[i]->setBrightness(brightness);
    clear(*displays[i]);
  }
}

void clearAll(TM1637Display* const* displays, size_t displayCount) {
  for (size_t i = 0; i < displayCount; ++i) {
    clear(*displays[i]);
  }
}

void renderNumbers(TM1637Display* const* displays,
                   size_t displayCount,
                   const int* values,
                   size_t valueCount,
                   int maxVisibleValue,
                   uint8_t defaultBrightness,
                   bool dynamicBrightnessEnabled) {
  for (size_t i = 0; i < displayCount; ++i) {
    if (i < valueCount && values[i] >= 0 && values[i] <= maxVisibleValue) {
      displays[i]->setBrightness(
        brightnessForValue(values[i], defaultBrightness, dynamicBrightnessEnabled)
      );
      displays[i]->showNumberDec(values[i], false);
    } else {
      displays[i]->setBrightness(defaultBrightness);
      clear(*displays[i]);
    }
  }
}

}  // namespace DisplayBoard
