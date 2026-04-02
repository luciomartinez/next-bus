#include "RemoteControl.h"

#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>

namespace RemoteControl {

namespace {

const char* const preferencesNamespace = "next-bus";
const char* const enabledKey = "enabled";
const uint16_t httpPort = 80;

Preferences preferences;
WebServer server(httpPort);
bool enabled = true;
bool enabledLoaded = false;
bool changed = false;
bool serverStarted = false;
String mdnsHostname;

void persistEnabledState() {
  preferences.putBool(enabledKey, enabled);
}

void setEnabledState(bool newValue) {
  if (enabled == newValue) {
    return;
  }

  enabled = newValue;
  persistEnabledState();
  changed = true;
  Serial.printf("Remote control set system %s\n", enabled ? "ON" : "OFF");
}

String statusJson() {
  String json = "{";
  json += "\"enabled\":";
  json += enabled ? "true" : "false";
  json += ",\"ip\":\"";
  json += WiFi.localIP().toString();
  json += "\",\"url\":\"";
  json += localUrl();
  json += "\"}";
  return json;
}

void handleRoot() {
  String body;
  body += "next_bus control\n";
  body += "on: ";
  body += localUrl();
  body += "/on\n";
  body += "off: ";
  body += localUrl();
  body += "/off\n";
  body += "toggle: ";
  body += localUrl();
  body += "/toggle\n";
  body += "status: ";
  body += localUrl();
  body += "/status\n";
  server.send(200, "text/plain", body);
}

void ensureServerStarted() {
  if (serverStarted || WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (mdnsHostname.length() > 0) {
    if (MDNS.begin(mdnsHostname.c_str())) {
      MDNS.addService("http", "tcp", httpPort);
      Serial.printf("mDNS started at http://%s.local\n", mdnsHostname.c_str());
    } else {
      Serial.println("Failed to start mDNS.");
    }
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/on", HTTP_GET, []() {
    setEnabledState(true);
    server.send(200, "application/json", statusJson());
  });
  server.on("/off", HTTP_GET, []() {
    setEnabledState(false);
    server.send(200, "application/json", statusJson());
  });
  server.on("/toggle", HTTP_GET, []() {
    setEnabledState(!enabled);
    server.send(200, "application/json", statusJson());
  });
  server.on("/status", HTTP_GET, []() {
    server.send(200, "application/json", statusJson());
  });
  server.onNotFound([]() {
    server.send(404, "application/json", "{\"error\":\"not_found\"}");
  });
  server.begin();
  serverStarted = true;
  Serial.printf("HTTP control ready at %s\n", localUrl().c_str());
}

}  // namespace

void begin(const char* hostname, bool defaultEnabled) {
  if (!enabledLoaded) {
    preferences.begin(preferencesNamespace, false);
    enabled = preferences.getBool(enabledKey, defaultEnabled);
    enabledLoaded = true;
    changed = false;
  }

  mdnsHostname = hostname != nullptr ? hostname : "";
  ensureServerStarted();
}

void handleClient() {
  ensureServerStarted();
  if (serverStarted) {
    server.handleClient();
  }
}

bool isEnabled() {
  return enabled;
}

bool stateChanged() {
  return changed;
}

void clearStateChanged() {
  changed = false;
}

String localUrl() {
  if (mdnsHostname.length() > 0) {
    return String("http://") + mdnsHostname + ".local";
  }
  return String("http://") + WiFi.localIP().toString();
}

}  // namespace RemoteControl
