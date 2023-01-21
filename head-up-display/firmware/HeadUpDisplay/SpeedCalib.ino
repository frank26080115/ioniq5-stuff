#include "HeadUpDisplay.h"

bool speedcalib_active = false;
bool speedcalib_log = false;

float speedcalib_convert(float rpm)
{
    rpm *= hud_settings.speed_multiplier;
    float frpm = rpm;
    return frpm / ((float)KMH2MPH_DIV);
}

float speedcalib_validate(int32_t rpm, int32_t kmh)
{
    float mph1 = speedcalib_convert(rpm);
    float mph2 = kmh * KMH2MPH_E6;
    mph2 /= KMH2MPH_DIV;
    return mph1 - mph2;
}

void speedcalib_task(uint32_t now)
{
    static uint32_t kmh_buf[2];
    static uint32_t rpm_buf[2];
    static uint8_t  idx = 0;
    static uint32_t save_timer = 0;
    static uint32_t kmh_max = 0;
    static int32_t  m_lpf = -1;
    static uint32_t saved_rpm;
    static uint32_t saved_kmh;

    static uint32_t last_dbg_time = 0;

    if (speedcalib_active == false && hud_settings.speed_multiplier != 0) {
        return;
    }

    if (hud_settings.speed_multiplier == 0) {
        speedcalib_active |= true;
    }

    if (save_timer != 0 && m_lpf > 0)
    {
        if ((now - save_timer) >= 3000)
        {
            hud_settings.speed_multiplier = lpf_read(m_lpf, 100);
            hud_settings.speed_kmh_max = kmh_max;
            hud_settings.speed_calib_rpm = saved_rpm;
            hud_settings.speed_calib_kmh = saved_kmh;
            save_timer = 0;
            settings_save();
            Serial.printf("[%u]: Speed Calib Save: %0.2f , %u , %u , %0.1f\r\n", now, hud_settings.speed_multiplier, saved_rpm, saved_kmh, speedcalib_validate(saved_rpm, saved_kmh));
        }
    }
    else if (kmh_max == 0)
    {
        kmh_max = hud_settings.speed_kmh_max;
    }

    if (car_data.speed_kmh < 56)
    {
        return;
    }

    idx = idx == 0 ? 1 : 0;
    kmh_buf[idx] = car_data.speed_kmh;
    rpm_buf[idx] = car_data.rpm;

    if (kmh_buf[0] == kmh_buf[1])
    {
        uint8_t nidx = idx == 0 ? 1 : 0;
        uint32_t kmh_used = kmh_buf[nidx];
        uint32_t rpm_used = rpm_buf[nidx]; 
        uint32_t mph = kmh_used * KMH2MPH_E6;
        uint32_t multiplier = rpm_used / mph;
        lpf_update(multiplier, &m_lpf, 10, 100);
        saved_kmh = kmh_used;
        saved_rpm = rpm_used;
        if (kmh_used > kmh_max) {
            // speed is faster than what we used before, so be should save this better value
            kmh_max = kmh_used;
            save_timer = now;
        }
        else if (kmh_used >= 96 && save_timer == 0)
        {
            // speed is very fast, and it has been a while, we can save another filtered value
            save_timer = now;
        }
    }
}
