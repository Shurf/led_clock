#include "HttpControl.h"
#include <esp_wifi.h>

void HttpControl::readMacAddress(){
    uint8_t baseMac[6];
    char macAddressCharArray[128];
    esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
    if (ret == ESP_OK) {
        sprintf(macAddressCharArray, "%02x:%02x:%02x:%02x:%02x:%02x",
                    baseMac[0], baseMac[1], baseMac[2],
                    baseMac[3], baseMac[4], baseMac[5]);
        baseMacString = macAddressCharArray;
    } else {
        Serial.println("Failed to read MAC address");
    }
}

HttpControl::HttpControl()
{
    readMacAddress();
    cacheMutex = xSemaphoreCreateMutex();
}

void HttpControl::start()
{
    xTaskCreatePinnedToCore(fetchTaskTrampoline, "httpFetch", 4096 * 2, this, 1, NULL, 0);
}

void HttpControl::fetchTaskTrampoline(void* ctx)
{
    static_cast<HttpControl*>(ctx)->fetchTask();
}

void HttpControl::fetchTask()
{
    while (true) {
        Arguments next;
        if (fetchOnce(next)) {
            if (xSemaphoreTake(cacheMutex, portMAX_DELAY) == pdTRUE) {
                cachedArguments = next;
                xSemaphoreGive(cacheMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(FETCH_INTERVAL_MS));
    }
}

bool HttpControl::fetchOnce(Arguments& out)
{
    HTTPClient http;
    http.setConnectTimeout(HTTP_CONNECT_TIMEOUT_MS);
    http.setTimeout(HTTP_REQUEST_TIMEOUT_MS);

    if (!http.begin(getCurrentLedProfileUrl + "?mac_address=" + baseMacString)) {
        Serial.println("HTTP begin failed");
        return false;
    }

    int code = http.GET();
    if (code != 200) {
        Serial.printf("HTTP GET failed: %d\n", code);
        http.end();
        return false;
    }

    String body = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        Serial.printf("JSON parse failed: %s\n", err.c_str());
        return false;
    }

    out.profileName = ((const char*)doc["profile_name"]);
    out.percentage = (int)doc["percentage"];
    out.enabled = (bool)doc["enabled"];
    out.useSecondary = (bool)doc["use_secondary"];
    out.red = (int)doc["red"];
    out.green = (int)doc["green"];
    out.blue = (int)doc["blue"];
    out.secondaryRed = (int)doc["secondary_red"];
    out.secondaryGreen = (int)doc["secondary_green"];
    out.secondaryBlue = (int)doc["secondary_blue"];
    out.secondaryRed2 = (int)doc["secondary_red2"];
    out.secondaryGreen2 = (int)doc["secondary_green2"];
    out.secondaryBlue2 = (int)doc["secondary_blue2"];

    return true;
}

Arguments HttpControl::getLedProfileFullParameters()
{
    Arguments snapshot;
    if (xSemaphoreTake(cacheMutex, portMAX_DELAY) == pdTRUE) {
        snapshot = cachedArguments;
        xSemaphoreGive(cacheMutex);
    }
    return snapshot;
}
