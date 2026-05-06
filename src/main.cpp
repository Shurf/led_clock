#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>

#ifndef OTA_PASSWORD
#define OTA_PASSWORD "ledclock"
#endif

#include "HttpControl.h"
#include "NeoPixelLed.h"
#include "ClockDialHour.h"
#include "ClockDialMinute.h"
#include "BacklightLed.h"
#include "secrets.h"
#include "FFT.h"
#include "LedManager.h"

#include <mutex>

// sound strip
// #define NEOPIXEL_LED_COUNT 88

#define NEOPIXEL_LED_PIN_MIDDLE_CIRCLE 25
#define NEOPIXEL_LED_PIN_OUTER_CIRCLE 26
#define NEOPIXEL_LED_PIN_INNER_CIRCLE 27
#define NEOPIXEL_LED_PIN_CENTER_DOT 33
#define NEOPIXEL_BRIGHTNESS 128
#define SOUND_BRIGHTNESS 128

#define NEOPIXEL_LED_RIGHT_COLUMN 32
#define NEOPIXEL_LED_LEFT_COLUMN 22
#define NEOPIXEL_LED_BOTTOM 23
#define NEOPIXEL_LED_TOP 21

std::mutex lock;
//#define NEOPIXEL_BRIGHTNESS 8
// #define NEOPIXEL_BRIGHTNESS 32



// NeoPixelLed * neoPixelLed = new NeoPixelLed(357, 255, NEOPIXEL_LED_PIN, false); // cross 
// NeoPixelLed * neoPixelLed = new NeoPixelLed(298, 255, NEOPIXEL_LED_PIN, true); // ankh 
// NeoPixelLed * neoPixelLed = new NeoPixelLed(24, 8, NEOPIXEL_LED_PIN, false); // circle
// NeoPixelLed * neoPixelLed = new NeoPixelLed(20, 255, NEOPIXEL_LED_PIN, false); // rabbit

//NeoPixelLed * neoPixelLedCenterDot = new NeoPixelLed(1, NEOPIXEL_BRIGHTNESS, NEOPIXEL_LED_PIN_CENTER_DOT, false);
ClockDialHour * clockHour = new ClockDialHour(NEOPIXEL_BRIGHTNESS, NEOPIXEL_LED_PIN_INNER_CIRCLE);
// NeoPixelLed * neoPixelLedMiddleCircle = new NeoPixelLed(24, NEOPIXEL_BRIGHTNESS, NEOPIXEL_LED_PIN_MIDDLE_CIRCLE, false);
ClockDialMinute * clockMinute = new ClockDialMinute(NEOPIXEL_BRIGHTNESS, NEOPIXEL_LED_PIN_OUTER_CIRCLE);

BacklightLed * rightBacklightLed = new BacklightLed(16, NEOPIXEL_BRIGHTNESS, NEOPIXEL_LED_RIGHT_COLUMN);
BacklightLed * leftBacklightLed = new BacklightLed(16, NEOPIXEL_BRIGHTNESS, NEOPIXEL_LED_LEFT_COLUMN);
BacklightLed * bottomBacklightLed = new BacklightLed(12, NEOPIXEL_BRIGHTNESS, NEOPIXEL_LED_BOTTOM);
BacklightLed * topBacklightLed = new BacklightLed(9, NEOPIXEL_BRIGHTNESS, NEOPIXEL_LED_TOP);

#define INTERVAL_MILLIS 1000

WiFiClientSecure client;
unsigned long lastMillis;
HttpControl* httpControl;

FFT* fft = new FFT();
LedManager* middleCircle = new LedManager(24, SOUND_BRIGHTNESS, NEOPIXEL_LED_PIN_MIDDLE_CIRCLE);
LedManager* centerDot = new LedManager(1, SOUND_BRIGHTNESS, NEOPIXEL_LED_PIN_CENTER_DOT);

int minimum = 32;
int maximum = 255;
int current = 128;
int step = 32; 

void otaLoop(void *context)
{
    while(true)
    {
        ArduinoOTA.handle();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void soundLoop(void *context)
{
    while(true)
    {
        float redPercentage;
        float greenPercentage;
        float bluePercentage;

        fft->calculatePercentages(redPercentage, greenPercentage, bluePercentage);   
        lock.lock();
        middleCircle->displayLeds(redPercentage, greenPercentage, bluePercentage);
        centerDot->displayLeds(redPercentage, greenPercentage, bluePercentage);
        //middleCircle->displayLeds(100.0, 100.0, 100.0);
        //centerDot->displayLeds(100.0, 100.0, 100.0);
        lock.unlock();
        
        vTaskDelay(20);   
    }
    
} 


void mainLoop(void *context) 
{
  int i = 0;
  unsigned long currentMillis;
  while(true) {

    tm localTime;
    currentMillis = millis();

    i++;
    if(i > 50)
    {
      i = 0;

      if(random(10) < 5)
        current = max(current - step, minimum);
      else
        current = min(current + step, maximum);

      lock.lock();  
      rightBacklightLed->display(current);
      leftBacklightLed->display(current);
      bottomBacklightLed->display(current);
      topBacklightLed->display(current);  
      lock.unlock();
    }

    if(currentMillis - lastMillis < INTERVAL_MILLIS)
    {
      vTaskDelay(1);
      continue;
    }

    lastMillis = currentMillis;

    getLocalTime(&localTime);


    auto arguments = httpControl->getLedProfileFullParameters();

    
    clockHour->setForegroundColor(arguments.red, arguments.green, arguments.blue);
    //clockHour->setBackgroundColor(arguments.secondaryRed, arguments.secondaryGreen, arguments.secondaryBlue);
    clockHour->setBackgroundColor(arguments.secondaryRed2, arguments.secondaryGreen2, arguments.secondaryBlue2);
    //clockHour->setTickColor(arguments.secondaryRed2, arguments.secondaryGreen2, arguments.secondaryBlue2);

    clockMinute->setForegroundColor(arguments.red, arguments.green, arguments.blue);
    //clockMinute->setBackgroundColor(arguments.secondaryRed, arguments.secondaryGreen, arguments.secondaryBlue);
    clockMinute->setBackgroundColor(0, 0, 0);
    clockMinute->setTickColor(arguments.secondaryRed2, arguments.secondaryGreen2, arguments.secondaryBlue2);

    lock.lock();
    clockHour->display(localTime.tm_hour % 12);
    clockMinute->display(localTime.tm_min);    
    lock.unlock();
  
    vTaskDelay(1);
  }
  
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Setup started");

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WIFIPASSWORD);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("WIFI Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname("led-clock");
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA
    .onStart([]() { Serial.println("OTA start"); })
    .onEnd([]() { Serial.println("OTA end"); })
    .onError([](ota_error_t e) { Serial.printf("OTA error: %u\n", e); });
  ArduinoOTA.begin();

  lastMillis = millis();

  httpControl = new HttpControl();
  httpControl->start();
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ","CET-1CEST,M3.5.0,M10.5.0/3",1);
  tzset();

  xTaskCreatePinnedToCore(mainLoop, "mainLoop", 4096*16, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(soundLoop, "soundLoop", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(otaLoop, "otaLoop", 4096, NULL, 1, NULL, 0);

  Serial.println("Setup finished");
}

void loop()
{

}


