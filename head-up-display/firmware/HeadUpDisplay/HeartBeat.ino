#include "HeadUpDisplay.h"

extern bool battlog_fileReady;
extern bool obd_isResponding;

CRGB heartbeat_strip[1];
CLEDController *heartbeat_ctrler;
bool heartbeat_isOn = false;

void heartbeat_init()
{
    pinMode(HUD_PIN_HEART_PWR, OUTPUT);
    digitalWrite(HUD_PIN_HEART_PWR, LOW);
    heartbeat_ctrler = &FastLED.addLeds<DOTSTAR, HUD_PIN_HEART_DATA, HUD_PIN_HEART_CLK, BGR>(heartbeat_strip, 1);
    heartbeat_strip[0] = CRGB::Black;
}

void heartbeat_task(uint32_t now)
{
    static uint8_t tick = 0;
    uint32_t now_mod = now % 1000;
    if (now_mod < 200)
    {
        if (heartbeat_isOn == false)
        {
            uint8_t brite = 128;
            tick++;
            if (obd_isResponding == false)
            {
                if (battlog_fileReady != false) {
                    heartbeat_strip[0] = CRGB::Purple;
                }
                else {
                    heartbeat_strip[0] = CRGB::Red;
                }
            }
            else if (car_data.ignition == false)
            {
                if (battlog_fileReady != false) {
                    heartbeat_strip[0] = CRGB::Green;
                }
                else {
                    heartbeat_strip[0] = CRGB::Blue;
                }
            }
            else if (car_data.ignition != false)
            {
                if (battlog_fileReady != false && (tick % 2) == 0) {
                    heartbeat_strip[0] = CRGB::Green;
                }
                else {
                    heartbeat_strip[0] = CRGB::White;
                }
            }
            heartbeat_ctrler->showLeds(brite);
            heartbeat_isOn = true;
        }
    }
    else
    {
        if (heartbeat_isOn != false)
        {
            heartbeat_strip[0] = CRGB::Black;
            heartbeat_ctrler->showLeds(0);
            heartbeat_isOn = false;
        }
    }
}
