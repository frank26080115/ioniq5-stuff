#include "HeadUpDisplay.h"

#include <DebuggingSerial.h>
#include <SerialCmdLine.h>
#include <WebSocketPrinter.h>
#include <MultiDestPrinter.h>
#include <AsyncAdc.h>
#include <isotp.h>
//#include <Adafruit_DotStar.h>
#include <FastLED.h>

#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "esp_task_wdt.h"

TaskHandle_t loop2task;

vehicle_data_t car_data = {0};
settings_t hud_settings;
uint8_t hud_state = HUDSTATE_INIT;

uint8_t hud_animation = HUDANI_OFF, hud_animation_queue = HUDANI_OFF;
int32_t hud_aniStep, hud_aniDelay;
uint32_t hud_offTime;

MultiDestPrinter log_printer;

DebuggingSerial dbg_ser(&Serial);

void setup()
{
    Serial.begin(115200);
    settings_load();

    bringup_tests();

    xTaskCreatePinnedToCore(
                loop2,       /* Task function. */
                "loop2",     /* name of task. */
                8192,        /* Stack size of task */
                NULL,        /* parameter of the task */
                1,           /* priority of the task */
                &loop2task,  /* Task handle to keep track of created task */
                ARDUINO_RUNNING_CORE == 0 ? 1 : 0 /* use the other core */
                );  
}

void loop()
{
    uint32_t now = millis();
}

void loop2(void* pvParameters)
{
    while (true)
    {
        

        esp_task_wdt_reset();
        vTaskDelay(5 + hud_aniDelay);
    }
}

void state_machine()
{
    uint32_t now = millis();

    if (hud_state == HUDSTATE_INIT)
    {
        if (car_data.rpm > 0)
        {
            hud_state = HUDSTATE_MOVING;
            hud_animation = HUDANI_SPEEDOMETER;
            hud_animation_queue = HUDANI_OFF;
        }
        else if (car_data.ignition != false)
        {
            hud_state = HUDSTATE_IGNITION;
            hud_animation = HUDANI_INTRO;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
        }
        else if (now >= 30 * 60 * 1000)
        {
            hud_state = HUDSTATE_OFF;
            hud_offTime = millis();
        }
        else if (now >= 2000 && car_data.ignition == false)
        {
            hud_state = HUDSTATE_IDLECHECK;
            hud_animation = HUDANI_VOLTMETER_FADEIN;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
        }
    }
    else if (hud_state == HUDSTATE_IDLECHECK)
    {
        if (car_data.rpm > 0 || car_data.ignition != false)
        {
            hud_state = HUDSTATE_MOVING;
            hud_animation = HUDANI_VOLTMETER_FADEOUT;
            hud_animation_queue = HUDANI_FADEIN;
            hud_aniStep = 0;
        }
        else if (now >= 30 * 60 * 1000 && car_data.ignition == false)
        {
            hud_state = HUDSTATE_OFF;
            hud_offTime = millis();
        }
    }
    else if (hud_state == HUDSTATE_IGNITION)
    {
        if (car_data.rpm > 0)
        {
            hud_state = HUDSTATE_MOVING;
            hud_animation = HUDANI_FADEIN;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
        }
        else if (car_data.ignition == false)
        {
            hud_state = HUDSTATE_OFF;
            hud_offTime = millis();
            hud_animation = HUDANI_VOLTMETER_FADEIN;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
        }
    }
    else if (hud_state == HUDSTATE_MOVING)
    {
        if (car_data.ignition == false)
        {
            hud_state = HUDSTATE_OFF;
            hud_offTime = millis();
            hud_animation = HUDANI_FADEOUT;
            hud_animation_queue = HUDANI_VOLTMETER_FADEIN;
            hud_aniStep = 0;
        }
    }
    else if (hud_state == HUDSTATE_OFF)
    {
        if ((now - hud_offTime) >= 5000 && hud_animation == HUDANI_VOLTMETER) {
            hud_animation = HUDANI_VOLTMETER_FADEOUT;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
        }
        if (car_data.ignition != false)
        {
            if (battlog_fileReady == false) // do not restart if logging is active
            {
                ESP.restart();
                // on restart, the next read of ignition status will trigger the animations
                // restart will reset the millis() timestamp
                // restart will kick the Wi-Fi on again
            }
        }
    }
}