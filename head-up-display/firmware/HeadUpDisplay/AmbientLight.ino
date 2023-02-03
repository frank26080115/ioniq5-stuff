#include "HeadUpDisplay.h"

#define AMBLIGHT_LPF_DIV    1000

bool amblight_dump = true;
uint32_t amblight_dump_time = 0;

int32_t amblight_val_x1000 = -1;
int32_t amblight_val, amblight_raw;
int16_t amblight_resbrite = -1;

void amblight_init()
{
    int8_t channel = digitalPinToAnalogChannel(HUD_PIN_AMBLIGHT);

    adcAttachPin(HUD_PIN_AMBLIGHT);
    adcStart(HUD_PIN_AMBLIGHT);
    Serial.printf("Amblight pin %u chan %d\r\n", HUD_PIN_AMBLIGHT, channel);
}

void amblight_task()
{
    if (adcBusy(HUD_PIN_AMBLIGHT) == false)
    {
        amblight_raw = adcEnd(HUD_PIN_AMBLIGHT);
        lpf_update(amblight_raw, &amblight_val_x1000, hud_settings.amblight_filter, AMBLIGHT_LPF_DIV);
        amblight_val = lpf_read(amblight_val_x1000, AMBLIGHT_LPF_DIV);
        adcStart(HUD_PIN_AMBLIGHT);

        if (amblight_dump)
        {
            uint32_t now;
            if (((now = millis()) - amblight_dump_time) >= 200)
            {
                Serial.printf("amblight[%u]: %u , %u , %u\r\n", now, amblight_raw, amblight_val, amblight_get());
            }
        }
    }
}

int16_t amblight_calc(int32_t x)
{
    double xd = x;
    double first_remap = mapd(xd, hud_settings.amblight_low, hud_settings.amblight_high, 0, 1);
    first_remap = (first_remap < 0) ? 0 : (first_remap > 1 ? 1 : first_remap);
    double curve = hud_settings.amblight_expo;
    double adjusted = expo_curve(first_remap, curve / 100.0);
    double brite = mapd(adjusted, 0, 1, hud_settings.amblight_min, 256);
    int britei = lround(brite);
    britei = britei < hud_settings.amblight_min ? hud_settings.amblight_min : britei;
    britei = britei >= 255 ? 255 : britei;
    return britei;
}

int16_t amblight_get()
{
    while (amblight_val_x1000 < 0) {
        amblight_task();
    }
    return amblight_resbrite = amblight_calc(amblight_val);
}

int32_t amblight_getRaw()
{
    return amblight_raw;
}

int32_t amblight_getFiltered()
{
    return amblight_val;
}

void amblight_print(Print* p)
{
    p->printf("%d, %d, %d,", amblight_raw, amblight_val, amblight_get());
}
