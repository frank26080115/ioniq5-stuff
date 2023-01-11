#include "HeadUpDisplay.h"

float    spdprecict_buff_speed[3] = { 0, 0, 0, };
uint32_t spdprecict_buff_time [3] = { 0, 0, 0, };

void spdpredict_convertSubmit()
{
    uint32_t now = millis();
    float mph;
    //if (obd_poll_mode == OBDPOLLMODE_EXTRAFAST)
    //{
    //    mph = car_data.speed_kmh * KMH2MPH_E6;
    //    mph /= KMH2MPH_DIV;
    //}
    //else
    {
        mph = speedcalib_convert(car_data.rpm);
    }
    spdpredict_submit(mph, now);
}

void spdpredict_submit(float mph, uint32_t t)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        spdprecict_buff_speed[i] = spdprecict_buff_speed[i + 1];
        spdprecict_buff_time [i] = spdprecict_buff_time [i + 1];
    }
    spdprecict_buff_speed[2] = mph;
    spdprecict_buff_time [2] = t;
}

float spdpredict_get(uint32_t t)
{
    float mph_latest = spdprecict_buff_speed[2];
    float mph_max = mph_latest + hud_settings.spdpredict_slew;
    float mph_min = mph_latest - hud_settings.spdpredict_slew;
    mph_min = mph_min < 0 ? 0 : mph_min;

    float dt_now    = t - spdprecict_buff_time[2];
    float dv_latest = mph_latest - spdprecict_buff_speed[1];
    float dv_prev   = spdprecict_buff_speed[1] - spdprecict_buff_speed[0];
    float dt_latest = spdprecict_buff_time [2] - spdprecict_buff_time [1];
    float dt_prev   = spdprecict_buff_time [1] - spdprecict_buff_time [0];
    float da        = (dv_latest / dt_latest) - (dv_prev / dt_prev);
    float dv_new    = dv_latest  + (da     * dt_now * hud_settings.spdpredict_jerk);
    float v_new     = mph_latest + (dv_new * dt_now * hud_settings.spdpredict_accel);

    v_new = v_new > mph_max ? mph_max : v_new;
    v_new = v_new < mph_min ? mph_min : v_new;

    return v_new;
}
