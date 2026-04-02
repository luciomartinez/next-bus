#include "DisplayBoard.h"
#include "RemoteControl.h"
#include "TrafiklabApi.h"
#include "WifiSupport.h"
#include "secrets.h"

const char* DEVICE_HOSTNAME = "next-bus";
const unsigned long SERIAL_BAUD_RATE = 115200;
const unsigned long WIFI_TIMEOUT_MS = 20000;
const unsigned long WIFI_RETRY_DELAY_MS = 500;
const unsigned long WIFI_RESET_DELAY_MS = 250;
const unsigned long UPDATE_INTERVAL_MS = 30000;
const unsigned long IDLE_LOOP_DELAY_MS = 25;
const uint8_t DISPLAY_BRIGHTNESS = 7;
const bool DYNAMIC_DISPLAY_BRIGHTNESS = true;
const int MAX_DISPLAY_MINUTES = 59;

// Display 1
#define CLK1 5
#define DIO1 4

// Display 2
#define CLK2 18
#define DIO2 19

// Display 3
#define CLK3 21
#define DIO3 22

TM1637Display display1(CLK1, DIO1);
TM1637Display display2(CLK2, DIO2);
TM1637Display display3(CLK3, DIO3);
TM1637Display* displays[] = {&display1, &display2, &display3};

constexpr size_t DISPLAY_COUNT = sizeof(displays) / sizeof(displays[0]);

const TrafiklabApi::DisplayTarget displayTargets[DISPLAY_COUNT] = {
  {TrafiklabApi::tollareTorgAreaId, "414", "Slussen", "Tollare torg 414 -> Slussen"},
  {TrafiklabApi::hedenstromsVagAreaId, "442", "Slussen", "Hedenstroms vag 442 -> Slussen"},
  {TrafiklabApi::tollareTorgAreaId, "442X", "Glasbruksgatan", "Tollare torg 442X -> Glasbruksgatan"}
};

bool missingApiKeyReported = false;
bool lastRenderedEnabledState = true;
unsigned long lastUpdateAttemptMs = 0;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  DisplayBoard::initialize(displays, DISPLAY_COUNT, DISPLAY_BRIGHTNESS);
  TrafiklabApi::configureTimezone();
  lastRenderedEnabledState = true;
}

void loop() {
  if (!WifiSupport::ensureConnection(
        WIFI_SSID,
        WIFI_PASSWORD,
        WIFI_TIMEOUT_MS,
        WIFI_RETRY_DELAY_MS,
        WIFI_RESET_DELAY_MS)) {
    DisplayBoard::clearAll(displays, DISPLAY_COUNT);
    return;
  }

  RemoteControl::begin(DEVICE_HOSTNAME, true);
  RemoteControl::handleClient();

  if (RemoteControl::stateChanged()) {
    lastRenderedEnabledState = !RemoteControl::isEnabled();
    lastUpdateAttemptMs = 0;
    RemoteControl::clearStateChanged();
  }

  if (!RemoteControl::isEnabled()) {
    if (lastRenderedEnabledState) {
      DisplayBoard::clearAll(displays, DISPLAY_COUNT);
      lastRenderedEnabledState = false;
    }
    delay(IDLE_LOOP_DELAY_MS);
    return;
  }

  if (!lastRenderedEnabledState) {
    lastUpdateAttemptMs = 0;
    lastRenderedEnabledState = true;
  }

  if (!TrafiklabApi::hasConfiguredApiKey()) {
    if (!missingApiKeyReported) {
      Serial.println("Set TRAFIKLAB_API_KEY in secrets.h before using the Trafiklab APIs.");
      missingApiKeyReported = true;
    }
    DisplayBoard::clearAll(displays, DISPLAY_COUNT);
    delay(IDLE_LOOP_DELAY_MS);
    return;
  }

  const unsigned long now = millis();
  if (lastUpdateAttemptMs != 0 && (now - lastUpdateAttemptMs) < UPDATE_INTERVAL_MS) {
    delay(IDLE_LOOP_DELAY_MS);
    return;
  }

  lastUpdateAttemptMs = now;
  int nextBusMinutes[DISPLAY_COUNT] = {-1, -1, -1};
  TrafiklabApi::fetchDisplayMinutes(
    displayTargets,
    DISPLAY_COUNT,
    nextBusMinutes,
    MAX_DISPLAY_MINUTES
  );
  DisplayBoard::renderNumbers(
    displays,
    DISPLAY_COUNT,
    nextBusMinutes,
    DISPLAY_COUNT,
    MAX_DISPLAY_MINUTES,
    DISPLAY_BRIGHTNESS,
    DYNAMIC_DISPLAY_BRIGHTNESS
  );
  delay(IDLE_LOOP_DELAY_MS);
}
