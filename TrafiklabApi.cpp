#include "TrafiklabApi.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "secrets.h"

namespace TrafiklabApi {

const char* const tollareTorgAreaId = "740076054";
const char* const hedenstromsVagAreaId = "740076458";

namespace {

const char* const apiBaseUrl = "https://realtime-api.trafiklab.se/v1";
const char* const timezone = "CET-1CEST,M3.5.0/2,M10.5.0/3";
const size_t departuresJsonCapacity = 32768;

bool parseIsoTimestamp(const char* value, time_t& parsedTime) {
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;

  int parts = sscanf(value, "%d-%d-%dT%d:%d:%d",
                     &year, &month, &day, &hour, &minute, &second);
  if (parts != 6) {
    parts = sscanf(value, "%d-%d-%dT%d:%d",
                   &year, &month, &day, &hour, &minute);
    if (parts != 5) {
      return false;
    }
    second = 0;
  }

  struct tm localTime = {};
  localTime.tm_year = year - 1900;
  localTime.tm_mon = month - 1;
  localTime.tm_mday = day;
  localTime.tm_hour = hour;
  localTime.tm_min = minute;
  localTime.tm_sec = second;
  localTime.tm_isdst = -1;

  parsedTime = mktime(&localTime);
  return parsedTime != static_cast<time_t>(-1);
}

bool fetchJson(const String& url, DynamicJsonDocument& doc) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  if (!http.begin(client, url)) {
    Serial.println("Failed to initialize HTTP client.");
    return false;
  }

  const int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP error %d for %s\n", httpCode, url.c_str());
    const String errorBody = http.getString();
    if (errorBody.length() > 0) {
      Serial.println(errorBody);
    }
    http.end();
    return false;
  }

  const String response = http.getString();
  http.end();

  DeserializationError jsonError = deserializeJson(doc, response);
  if (jsonError) {
    Serial.printf("JSON parse error: %s\n", jsonError.c_str());
    return false;
  }

  return true;
}

bool isCanceled(JsonObject departure) {
  return (departure["canceled"] | false) ||
         (departure["is_cancelled"] | false) ||
         strcmp(departure["state"] | "", "CANCELLED") == 0;
}

bool matchesDirection(JsonObject departure, const char* expectedDirection) {
  const char* routeDirection = departure["route"]["direction"] | "";
  const char* routeDestination = departure["route"]["destination"]["name"] | "";
  const char* legacyDestination = departure["destination"] | "";

  return strcmp(routeDirection, expectedDirection) == 0 ||
         strcmp(routeDestination, expectedDirection) == 0 ||
         strcmp(legacyDestination, expectedDirection) == 0;
}

bool extractDepartureMinutes(JsonObject departure, time_t queryTime, int& minutes) {
  const char* realtime = departure["realtime"] | "";
  const char* scheduled = departure["scheduled"] | "";
  const char* departureTimeText = realtime[0] != '\0' ? realtime : scheduled;
  if (departureTimeText[0] == '\0') {
    return false;
  }

  time_t departureTime;
  if (!parseIsoTimestamp(departureTimeText, departureTime)) {
    return false;
  }

  const double secondsUntilDeparture = difftime(departureTime, queryTime);
  minutes = static_cast<int>(ceil(secondsUntilDeparture / 60.0));
  if (minutes < 0) {
    minutes = 0;
  }

  return true;
}

bool findMinutesForTarget(JsonArray departures,
                          time_t queryTime,
                          const DisplayTarget& target,
                          int maxDisplayMinutes,
                          int& minutes) {
  for (JsonObject departure : departures) {
    const char* transportMode = departure["route"]["transport_mode"] | "";
    if (strcmp(transportMode, "BUS") != 0) {
      continue;
    }

    if (isCanceled(departure)) {
      continue;
    }

    const char* line = departure["route"]["designation"] | "";
    if (strcmp(line, target.line) != 0) {
      continue;
    }

    if (!matchesDirection(departure, target.direction)) {
      continue;
    }

    if (!extractDepartureMinutes(departure, queryTime, minutes)) {
      continue;
    }

    // Only the next matching departure matters for each display target.
    if (minutes > maxDisplayMinutes) {
      return false;
    }

    return true;
  }

  return false;
}

bool fetchAreaDisplayMinutes(const char* areaId,
                             const DisplayTarget* targets,
                             size_t targetCount,
                             int* minutesBuffer,
                             int maxDisplayMinutes) {
  DynamicJsonDocument doc(departuresJsonCapacity);
  const String url = String(apiBaseUrl) +
                     "/departures/" + String(areaId) +
                     "?key=" + String(TRAFIKLAB_API_KEY);

  if (!fetchJson(url, doc)) {
    return false;
  }

  const char* queryTimeText = doc["query"]["queryTime"] | doc["timestamp"] | "";
  if (queryTimeText[0] == '\0') {
    Serial.println("Departures response did not contain query.queryTime.");
    return false;
  }

  time_t queryTime;
  if (!parseIsoTimestamp(queryTimeText, queryTime)) {
    Serial.printf("Failed to parse query time: %s\n", queryTimeText);
    return false;
  }

  JsonArray departures = doc["departures"].as<JsonArray>();
  if (departures.isNull()) {
    Serial.println("Departures response did not contain a departures array.");
    return false;
  }

  for (size_t i = 0; i < targetCount; ++i) {
    if (strcmp(targets[i].areaId, areaId) != 0) {
      continue;
    }

    int minutes = -1;
    if (findMinutesForTarget(departures, queryTime, targets[i], maxDisplayMinutes, minutes)) {
      minutesBuffer[i] = minutes;
      Serial.printf("Display %u: %s in %d min\n",
                    static_cast<unsigned>(i + 1),
                    targets[i].label,
                    minutes);
    } else {
      Serial.printf("Display %u: no departure within %d min for %s\n",
                    static_cast<unsigned>(i + 1),
                    maxDisplayMinutes,
                    targets[i].label);
    }
  }

  return true;
}

}  // namespace

void configureTimezone() {
  setenv("TZ", timezone, 1);
  tzset();
}

bool hasConfiguredApiKey() {
  return TRAFIKLAB_API_KEY[0] != '\0' &&
         strcmp(TRAFIKLAB_API_KEY, "REPLACE_WITH_YOUR_TRAFIKLAB_API_KEY") != 0;
}

bool fetchDisplayMinutes(const DisplayTarget* targets,
                         size_t targetCount,
                         int* minutesBuffer,
                         int maxDisplayMinutes) {
  bool allRequestsSucceeded = true;

  for (size_t i = 0; i < targetCount; ++i) {
    bool areaAlreadyFetched = false;
    for (size_t j = 0; j < i; ++j) {
      if (strcmp(targets[i].areaId, targets[j].areaId) == 0) {
        areaAlreadyFetched = true;
        break;
      }
    }

    if (areaAlreadyFetched) {
      continue;
    }

    if (!fetchAreaDisplayMinutes(
          targets[i].areaId,
          targets,
          targetCount,
          minutesBuffer,
          maxDisplayMinutes)) {
      allRequestsSucceeded = false;
    }
  }

  return allRequestsSucceeded;
}

}  // namespace TrafiklabApi
