#pragma once

#include <Arduino.h>

namespace TrafiklabApi {

extern const char* const tollareTorgAreaId;
extern const char* const hedenstromsVagAreaId;

struct DisplayTarget {
  const char* areaId;
  const char* line;
  const char* direction;
  const char* label;
};

void configureTimezone();
bool hasConfiguredApiKey();
bool fetchDisplayMinutes(const DisplayTarget* targets,
                         size_t targetCount,
                         int* minutesBuffer,
                         int maxDisplayMinutes);

}  // namespace TrafiklabApi
