#include "HeadUpDisplay.h"

// index 0 is oldest, index 2 is latest
float    spdprecict_buff_speed[3] = { 0, 0, 0, };
uint32_t spdprecict_buff_time [3] = { 0, 0, 0, };

static SemaphoreHandle_t spdpredict_mutex = xSemaphoreCreateMutex();
#define SPDPREDICT_MUTEX_DLY 200

#ifdef ENABLE_SPEED_PREDICTION_JERK_FILTER
bool spdpredict_needCalcJerk = false;
#endif

void spdpredict_submit(float spd, uint32_t t)
{
    #ifndef DISABLE_SPEED_PREDICTION

    xSemaphoreTake(spdpredict_mutex, MS_TO_RTOS_TICKS(SPDPREDICT_MUTEX_DLY));

    if (t == spdprecict_buff_time[2]) {
        // submission is too recent, so just update the most recent speed
        spdprecict_buff_speed[2] = spd;
        xSemaphoreGive(spdpredict_mutex);
        return;
    }

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

    xSemaphoreGive(spdpredict_mutex);
    #endif
}

float spdpredict_get(uint32_t t)
{
    float v_new;

    xSemaphoreTake(spdpredict_mutex, MS_TO_RTOS_TICKS(SPDPREDICT_MUTEX_DLY));

    #ifdef ENABLE_SPEED_PREDICTION_JERK_FILTER
    static float jerk_filter = NAN;
    #endif
    static float prev_prediction = 0;
    static float prev_ret = 0;

    #ifdef DISABLE_SPEED_PREDICTION
    v_new = spdprecict_buff_speed[2];
    prev_prediction = v_new;
    xSemaphoreGive(spdpredict_mutex);
    return spdpredict_slewFilter(v_new);
    #endif

    #ifdef ENABLE_TEST_PRINT_SPEED_FAST
    //hud_settings.spdpredict_slew = 10000;
    hud_settings.spdpredict_slew = 35;
    hud_settings.spdpredict_factor = 1.0;
    hud_settings.spdpredict_jerkFilter = 0.0;
    #endif

    if (hud_settings.spdpredict_slew <= 0 || hud_settings.spdpredict_factor <= 0) {
        v_new = spdprecict_buff_speed[2];
        prev_prediction = v_new;
        xSemaphoreGive(spdpredict_mutex);
        return spdpredict_slewFilter(v_new);
    }

    float spd_latest = spdprecict_buff_speed[2];

    float dt_now = t - spdprecict_buff_time[2];
    if (dt_now <= 1) {
        v_new = spdprecict_buff_speed[2];
        prev_prediction = v_new;
        xSemaphoreGive(spdpredict_mutex);
        return spdpredict_slewFilter(v_new);
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

    xSemaphoreGive(spdpredict_mutex);

    // timeout for prediction, need new data
    if (dt_now >= 100) {
        return spdpredict_slewFilter(prev_prediction);
    }

    float accel1 = dv_latest / dt_latest; // previous acceleration
    float accel2 = da / dt_now; // predicted new acceleration based on jerk
    float dvdt_new = 0;

    // we are allowed to predict that acceleration is decreasing but not increasing
    if ((accel1 > 0 && accel2 > 0) || (accel1 < 0 && accel2 < 0)) {
        accel2 = 0;
    }
    dvdt_new = accel1 + accel2;

    float dv = dvdt_new * dt_now;
    float factor = hud_settings.spdpredict_factor;

    v_new = spd_latest + (dv * factor);
    v_new = v_new < 0 ? 0 : v_new;
    prev_prediction = v_new;
    return spdpredict_slewFilter(v_new);
}

float spdpredict_slewFilter(float v_new)
{
    static float prev_ret = 0;

    if (hud_settings.spdpredict_slew > 0)
    {
        float y = prev_ret;
        if (v_new > prev_ret + hud_settings.spdpredict_slew) {
            y = prev_ret + hud_settings.spdpredict_slew;
        }
        else if (v_new > prev_ret) {
            y = v_new;
        }
        else if (v_new < prev_ret - hud_settings.spdpredict_slew) {
            y = prev_ret - hud_settings.spdpredict_slew;
        }
        else if (v_new < prev_ret) {
            y = v_new;
        }
        v_new = y;
    }

    v_new = v_new < 0 ? 0 : v_new;
    prev_ret = v_new;
    return prev_ret;
}
