#include "WifiSupport.h"

#include <WiFi.h>

namespace WifiSupport {

namespace {

void resetRadio(unsigned long resetDelayMs) {
  WiFi.disconnect(true, true);
  delay(resetDelayMs);
  WiFi.mode(WIFI_STA);
  delay(resetDelayMs);
}

}  // namespace

bool ensureConnection(const char* ssid,
                      const char* password,
                      unsigned long timeoutMs,
                      unsigned long retryDelayMs,
                      unsigned long resetDelayMs) {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.printf("Connecting to WiFi \"%s\"", ssid);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  resetRadio(resetDelayMs);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  wl_status_t status = WiFi.status();
  while (status != WL_CONNECTED && (millis() - start) < timeoutMs) {
    delay(retryDelayMs);
    Serial.print(".");
    status = WiFi.status();
  }

  if (status == WL_CONNECTED) {
    Serial.printf(" connected, IP %s\n", WiFi.localIP().toString().c_str());
    return true;
  }

  Serial.printf(" failed (status %d).\n", static_cast<int>(status));
  return false;
}

}  // namespace WifiSupport
