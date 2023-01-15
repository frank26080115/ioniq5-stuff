#include "HeadUpDisplay.h"

#define AMBLIGHT_LPF_FACTOR 20
#define AMBLIGHT_LPF_DIV    1000

bool amblight_dump = false;
uint32_t amblight_dump_time = 0;

int32_t amblight_val_x1000 = -1;
int32_t amblight_val;
int16_t amblight_resbrite = -1;
int16_t amblight_firstRaw = -1;

void amblight_init()
{
    adcAttachPin(HUD_PIN_AMBLIGHT);
    adcStart(HUD_PIN_AMBLIGHT);
}

void amblight_task()
{
    if (adcBusy(HUD_PIN_AMBLIGHT) == false)
    {
        uint16_t x = adcEnd(HUD_PIN_AMBLIGHT);
        amblight_firstRaw = (amblight_firstRaw < 0) ? x : amblight_firstRaw;
        lpf_update(x, &amblight_val_x1000, AMBLIGHT_LPF_FACTOR, AMBLIGHT_LPF_DIV);
        amblight_val = lpf_read(amblight_val_x1000, AMBLIGHT_LPF_DIV);
        adcStart(HUD_PIN_AMBLIGHT);

        if (amblight_dump)
        {
            uint32_t now;
            if (((now = millis()) - amblight_dump_time) >= 200)
            {
                Serial.printf("amblight[%u]: %u , %u\r\n", now, amblight_val, amblight_get());
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
