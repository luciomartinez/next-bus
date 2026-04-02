#include "DisplayBoard.h"

namespace DisplayBoard {

namespace {

uint8_t brightnessForValue(int value, uint8_t defaultBrightness, bool dynamicBrightnessEnabled) {
  if (!dynamicBrightnessEnabled || value < 0) {
    return defaultBrightness;
  }

  if (value <= 10) {
    return 7;
  }
  if (value <= 14) {
    return 6;
  }
  if (value <= 19) {
    return 5;
  }
  if (value <= 24) {
    return 4;
  }
  if (value <= 29) {
    return 3;
  }
  if (value <= 39) {
    return 2;
  }
  if (value <= 59) {
    return 1;
  }

  return defaultBrightness;
}

void clear(TM1637Display& display) {
  uint8_t blankSegments[] = {0x00, 0x00, 0x00, 0x00};
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
