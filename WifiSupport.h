#pragma once

#include <Arduino.h>

namespace WifiSupport {

bool ensureConnection(const char* ssid,
                      const char* password,
                      unsigned long timeoutMs,
                      unsigned long retryDelayMs,
                      unsigned long resetDelayMs);

}  // namespace WifiSupport
