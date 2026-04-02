#pragma once

#include <Arduino.h>
#include <TM1637Display.h>

namespace DisplayBoard {

void initialize(TM1637Display* const* displays, size_t displayCount, uint8_t brightness);
void clearAll(TM1637Display* const* displays, size_t displayCount);
void renderNumbers(TM1637Display* const* displays,
                   size_t displayCount,
                   const int* values,
                   size_t valueCount,
                   int maxVisibleValue,
                   uint8_t defaultBrightness,
                   bool dynamicBrightnessEnabled);

}  // namespace DisplayBoard
