#include "HeadUpDisplay.h"

// index 0 is oldest, index 2 is latest
float    spdprecict_buff_speed[3] = { 0, 0, 0, };
uint32_t spdprecict_buff_time [3] = { 0, 0, 0, };

#ifdef ENABLE_SPEED_PREDICTION_JERK_FILTER
bool spdpredict_needCalcJerk = false;
#endif

void spdpredict_submit(float spd, uint32_t t)
{
    int i;
    // shift the history
    for (i = 0; i < 2; i++)
    {
        spdprecict_buff_speed[i] = spdprecict_buff_speed[i + 1];
        spdprecict_buff_time [i] = spdprecict_buff_time [i + 1];
    }
    // place new sample in most recent index
    spdprecict_buff_speed[i] = spd;
    spdprecict_buff_time [i] = t;
    #ifdef ENABLE_SPEED_PREDICTION_JERK_FILTER
    spdpredict_needCalcJerk = true;
    #endif
}

float spdpredict_get(uint32_t t)
{
    #ifdef ENABLE_SPEED_PREDICTION_JERK_FILTER
    static float jerk_filter = NAN;
    #endif
    static float prev_v = 0;

    #ifdef DISABLE_SPEED_PREDICTION
    prev_v = spdprecict_buff_speed[2];
    return spdprecict_buff_speed[2];
    #endif

    #ifdef ENABLE_TEST_PRINT_SPEED_FAST
    //hud_settings.spdpredict_slew = 10000;
    hud_settings.spdpredict_slew = 200;
    hud_settings.spdpredict_factor = 1.0;
    hud_settings.spdpredict_jerkFilter = 0.0;
    #endif

    if (hud_settings.spdpredict_slew <= 0 || hud_settings.spdpredict_factor <= 0) {
        prev_v = spdprecict_buff_speed[2];
        return spdprecict_buff_speed[2];
    }

    float spd_latest = spdprecict_buff_speed[2];
    float spd_max = spd_latest + hud_settings.spdpredict_slew;
    float spd_min = spd_latest - hud_settings.spdpredict_slew;
    spd_min = spd_min < 0 ? 0 : spd_min;

    float dt_now    = t - spdprecict_buff_time[2];
    if (dt_now <= 0) {
        prev_v = spdprecict_buff_speed[2];
        return spdprecict_buff_speed[2];
    }
    float dv_latest = spd_latest - spdprecict_buff_speed[1];
    float dv_prev   = spdprecict_buff_speed[1] - spdprecict_buff_speed[0];
    float dt_latest = spdprecict_buff_time [2] - spdprecict_buff_time [1];
    float dt_prev   = spdprecict_buff_time [1] - spdprecict_buff_time [0];

    float da;
    #ifdef ENABLE_SPEED_PREDICTION_JERK_FILTER
    if (hud_settings.spdpredict_jerkFilter > 0) // jerk filter enabled
    {
        if (isnan(jerk_filter)) {
            jerk_filter = da;
        }

        if (spdpredict_needCalcJerk != false)
        {
            da = (dv_latest / dt_latest) - (dv_prev / dt_prev);
            jerk_filter = (jerk_filter * hud_settings.spdpredict_jerkFilter) + (da * (1.0 - hud_settings.spdpredict_jerkFilter));
            spdpredict_needCalcJerk = false;
        }

        da = jerk_filter;
    }
    else // jerk filter disabled
    #endif
    {
        da = (dv_latest / dt_latest) - (dv_prev / dt_prev);
    }

    // timeout for prediction, need new data
    if (dt_now >= 100) {
        return prev_v;
    }

    float accel1 = dv_latest / dt_latest;
    float accel2 = da / dt_now;
    float dvdt_new = 0;

    // we are allowed to predict that acceleration is decreasing but not increasing
    if ((accel1 > 0 && accel2 > 0) || (accel1 < 0 && accel2 < 0)) {
        accel2 = 0;
    }
    dvdt_new = accel1 + accel2;

    float dv = dvdt_new * dt_now;
    float factor = hud_settings.spdpredict_factor;

    float v_new = spd_latest + (dv * factor);

    v_new = v_new > spd_max ? spd_max : v_new;
    v_new = v_new < spd_min ? spd_min : v_new;

    prev_v = v_new;

    return v_new;
}
