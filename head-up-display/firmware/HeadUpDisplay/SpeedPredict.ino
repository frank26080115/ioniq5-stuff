#include "HeadUpDisplay.h"

float    spdprecict_buff_speed[3] = { 0, 0, 0, };
uint32_t spdprecict_buff_time [3] = { 0, 0, 0, };

void spdpredict_submit(float spd, uint32_t t)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        spdprecict_buff_speed[i] = spdprecict_buff_speed[i + 1];
        spdprecict_buff_time [i] = spdprecict_buff_time [i + 1];
    }
    spdprecict_buff_speed[i] = spd;
    spdprecict_buff_time [i] = t;
}

float spdpredict_get(uint32_t t)
{
    float spd_latest = spdprecict_buff_speed[2];
    float spd_max = spd_latest + hud_settings.spdpredict_slew;
    float spd_min = spd_latest - hud_settings.spdpredict_slew;
    spd_min = spd_min < 0 ? 0 : spd_min;

    float dt_now    = t - spdprecict_buff_time[2];
    float dv_latest = spd_latest - spdprecict_buff_speed[1];
    float dv_prev   = spdprecict_buff_speed[1] - spdprecict_buff_speed[0];
    float dt_latest = spdprecict_buff_time [2] - spdprecict_buff_time [1];
    float dt_prev   = spdprecict_buff_time [1] - spdprecict_buff_time [0];
    float da        = (dv_latest / dt_latest) - (dv_prev / dt_prev);
    float dvdt_new  = (dv_latest / dt_latest) + (da / dt_now);
    float dv        = dvdt_new * dt_now;
    float v_new     = spd_latest + (dv * hud_settings.spdpredict_factor);

    v_new = v_new > spd_max ? spd_max : v_new;
    v_new = v_new < spd_min ? spd_min : v_new;

    return v_new;
}
