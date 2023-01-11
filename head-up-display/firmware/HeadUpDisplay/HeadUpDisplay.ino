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

TaskHandle_t loop2task;

vehicle_data_t car_data;
settings_t hud_settings;

MultiDestPrinter log_printer;

DebuggingSerial dbg_ser(&Serial);

//Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

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
        uint32_t now = millis();

        vTaskDelay(5);
    }
}