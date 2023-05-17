#pragma once

#include <ArduinoJson.h>

class WiFiClient;
struct owm_resp_onecall_t;
struct owm_resp_air_pollution_t;

DeserializationError deserializeOneCall(WiFiClient &json, 
                                        owm_resp_onecall_t &r);
DeserializationError deserializeAirQuality(WiFiClient &json, 
                                           owm_resp_air_pollution_t &r);
