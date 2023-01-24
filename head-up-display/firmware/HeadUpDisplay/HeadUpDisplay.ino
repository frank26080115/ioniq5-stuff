#include "HeadUpDisplay.h"

#include <DebuggingSerial.h>
#include <SerialCmdLine.h>
#include <WebSocketPrinter.h>
#include <MultiDestPrinter.h>
#include <StringPrinter.h>
#include <AsyncAdc.h>
#include <isotp.h>
#include <FastLED.h>

#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "esp_task_wdt.h"

TaskHandle_t loop2task;

vehicle_data_t car_data = {0};
settings_t hud_settings = {0};
uint8_t hud_state = HUDSTATE_INIT;

uint8_t hud_animation = HUDANI_OFF, hud_animation_queue = HUDANI_OFF;
int32_t hud_aniStep, hud_aniDelay;
uint32_t hud_offTime;

MultiDestPrinter log_printer;
StringPrinter log_cacher(1024 * 4);

DebuggingSerial dbg_ser(&Serial);
extern SerialCmdLine cmdline;

void setup()
{
    Serial.begin(115200);
    settings_load();

    bringup_tests();

    amblight_init();
    heartbeat_init();
    canbus_init();
    battlog_init();

    Serial.println("HUD all init done, spinning up 2nd thread");

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

    canbus_poll();
    obd_queryTask(now);

    cmdline.task();
    amblight_task();
    web_task(now);
}

void loop2(void* pvParameters)
{
    #define LOOP2_MIN_DELAY 5
    while (true)
    {
        uint32_t now = millis();

        state_machine(now);

        speedcalib_task(now);
        car_data.speed_mph = speedcalib_convert(spdpredict_get(now = millis()));

        hud_aniDelay = 0;
        strip_task(now);
        hud_aniDelay = (hud_aniDelay > 1) ? (hud_aniDelay - 1) : hud_aniDelay;

        esp_task_wdt_reset();

        if (hud_aniDelay > 0)
        {
            // since SD card writing is done in large chunks, do it only right after each frame of animation
            heartbeat_task(now = millis());
            battlog_task(now);
            // subtract the time needed for the battery logging SD card write, to keep frame rate consistent
            hud_aniDelay -= millis() - now;
            hud_aniDelay = hud_aniDelay < 0 ? 0 : hud_aniDelay;
        }

        vTaskDelay(LOOP2_MIN_DELAY + MS_TO_RTOS_TICKS(hud_aniDelay)); // keep animation frame rate, and do other tasks
    }
}

void state_machine(uint32_t now)
{
    if (hud_state == HUDSTATE_INIT)
    {
        if (car_data.rpm > 0)
        {
            hud_state = HUDSTATE_MOVING;
            hud_animation = HUDANI_SPEEDOMETER;
            hud_animation_queue = HUDANI_OFF;
            dbg_ser.printf("[%u]: SM init -> moving\r\n", now);
        }
        else if (car_data.ignition != false)
        {
            hud_state = HUDSTATE_IGNITION;
            hud_animation = HUDANI_INTRO;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
            dbg_ser.printf("[%u]: SM init -> ignition\r\n", now);
        }
        else if (now >= 30 * 60 * 1000)
        {
            hud_state = HUDSTATE_OFF;
            hud_offTime = now;
        }
        else if (now >= 2000 && car_data.ignition == false)
        {
            hud_state = HUDSTATE_IDLECHECK;
            hud_animation = HUDANI_VOLTMETER_FADEIN;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
            dbg_ser.printf("[%u]: SM init -> voltmeter\r\n", now);
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
            dbg_ser.printf("[%u]: SM voltmeter -> moving\r\n", now);
        }
        else if (now >= 30 * 60 * 1000 && car_data.ignition == false)
        {
            hud_state = HUDSTATE_OFF;
            hud_offTime = now;
            dbg_ser.printf("[%u]: SM idle -> off\r\n", now);
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
            dbg_ser.printf("[%u]: SM ignition -> moving\r\n", now);
        }
        else if (car_data.ignition == false)
        {
            hud_state = HUDSTATE_OFF;
            hud_offTime = now;
            hud_animation = HUDANI_VOLTMETER_FADEIN;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
            dbg_ser.printf("[%u]: SM ignition -> off\r\n", now);
        }
    }
    else if (hud_state == HUDSTATE_MOVING)
    {
        if (car_data.ignition == false)
        {
            hud_state = HUDSTATE_OFF;
            hud_offTime = now;
            hud_animation = HUDANI_FADEOUT;
            hud_animation_queue = HUDANI_VOLTMETER_FADEIN;
            hud_aniStep = 0;
            dbg_ser.printf("[%u]: SM moving -> off\r\n", now);
        }
    }
    else if (hud_state == HUDSTATE_OFF)
    {
        if ((now - hud_offTime) >= 5000 && hud_animation == HUDANI_VOLTMETER) {
            hud_animation = HUDANI_VOLTMETER_FADEOUT;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
            dbg_ser.printf("[%u]: SM voltmeter fadeout\r\n", now);
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