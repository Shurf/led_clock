#pragma once

#include "Arduino.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define MAX_OUTPUT_VALUE 255
#define PROFILE_NAME "LED1"
#define MAX_TRIES_COUNT 5
#define SLEEP_INTERVAL 1000
#define FETCH_INTERVAL_MS 10000
#define HTTP_CONNECT_TIMEOUT_MS 2000
#define HTTP_REQUEST_TIMEOUT_MS 2000

struct Arguments {
    String profileName;
    int red;
    int green;
    int blue;
    int percentage;
    bool is_current;
    int secondaryRed;
    int secondaryGreen;
    int secondaryBlue;
    int secondaryRed2;
    int secondaryGreen2;
    int secondaryBlue2;
    bool enabled;
    bool useSecondary;
};

class HttpControl
{
public:

    HttpControl();
    void start();
    Arguments getLedProfileFullParameters();

private:

    String baseMacString;
    String getCurrentLedProfileUrl = "http://led.haven/neon_led_control/led_profiles/current";

    Arguments cachedArguments{};
    SemaphoreHandle_t cacheMutex = nullptr;

    void readMacAddress();
    bool fetchOnce(Arguments& out);
    void fetchTask();
    static void fetchTaskTrampoline(void* ctx);
};
