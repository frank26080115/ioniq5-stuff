#include "HeadUpDisplay.h"

void strip_blank()
{
    int i;
    for (i = 0; i < LED_STRIP_SIZE; i++) {
        leds[i] = CRGB_BLACK();
    }
}

bool strip_hasAnyTicks()
{
    int i;
    for (i = 0; i < LED_STRIP_SIZE; i += SPEED_TICK_SPACING) {
        if (leds[i].r > 0 || leds[i].g > 0 || leds[i].b > 0) {
            return false;
        }
    }
    return true;
}

bool strip_isBlank()
{
    int i;
    for (i = 0; i < LED_STRIP_SIZE; i += 1) {
        if (leds[i].r > 0 || leds[i].g > 0 || leds[i].b > 0) {
            return false;
        }
    }
    return true;
}

bool strip_hasAllTicks()
{
    int i;
    for (i = 0; i < LED_STRIP_SIZE; i += SPEED_TICK_SPACING) {
        if (leds[i].b < hud_settings.ledbrite_tick) {
            return false;
        }
    }
    return true;
}

bool strip_isAllTicks()
{
    int i;
    for (i = 0; i < LED_STRIP_SIZE; i++)
    {
        if ((i % SPEED_TICK_SPACING) == 0)
        {
            if (leds[i].b < hud_settings.ledbrite_tick) {
                return false;
            }
        }
        else
        {
            if (leds[i].b > 0) {
                return false;
            }
        }
    }
    return true;
}

void strip_aniFadeInReport()
{
    if (hud_aniStep == 0) {
        dbg_ser.printf("[%u]: ANI fade-in %d start\r\n", millis(), hud_animation);
    }
}

void strip_aniFadeOutReport()
{
    if (hud_aniStep == 0) {
        dbg_ser.printf("[%u]: ANI fade-out %d start\r\n", millis(), hud_animation);
    }
}
