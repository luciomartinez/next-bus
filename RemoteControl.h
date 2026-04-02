#pragma once

#include <Arduino.h>

namespace RemoteControl {

void begin(const char* hostname, bool defaultEnabled);
void handleClient();
bool isEnabled();
bool stateChanged();
void clearStateChanged();
String localUrl();

}  // namespace RemoteControl
