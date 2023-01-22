#include "HeadUpDisplay.h"

extern bool obd_debug_dump;
extern uint32_t obd_debug_rate;
extern uint8_t obd_poll_mode;
extern bool obd_hasNewLog;
extern bool obd_hasNewSpeed;

extern bool speedcalib_log;
extern bool speedcalib_active;

void bringup_tests()
{
    //bringuptest_sdcard();
    //bringuptest_heartbeat();
    //bringuptest_canbusquery();
    //bringuptest_canbusspy();
    //bringuptest_canbusspeed();
    bringuptest_speedcalibration();
}

void bringuptest_sdcard()
{
    battlog_init();
    battlog_startNewLog();
    battlog_log();
    battlog_log();
    battlog_log();
    battlog_log();
    battlog_close();
}

void bringuptest_canbusspy()
{
    canbus_init();
    obd_debug_dump = true;
    obd_debug_rate = 10;
    Serial.println("testing CAN bus spying");
    while (true)
    {
        canbus_poll();
    }
}

void bringuptest_canbusquery()
{
    uint32_t now;
    dbg_ser.enabled = true;
    canbus_init();
    obd_debug_dump = true;
    obd_debug_rate = 10;
    obd_poll_mode = OBDPOLLMODE_SIMPLE;
    speedcalib_log = true;
    speedcalib_active = true;
    Serial.println("testing CAN bus polling");
    while (true)
    {
        canbus_poll();
        obd_queryTask(now = millis());
        if ((obd_hasNewLog != false && obd_poll_mode == OBDPOLLMODE_BATTLOG_LONG) || (obd_hasNewSpeed != false && obd_poll_mode == OBDPOLLMODE_SIMPLE))
        {
            obd_printLog(dynamic_cast<Print*>(&Serial));
            obd_hasNewLog = false;
            obd_hasNewSpeed = false;
        }
        obdstat_reportTask(now);
    }
}

void bringuptest_canbusspeed()
{
    uint32_t now;
    canbus_init();
    obd_poll_mode = OBDPOLLMODE_SIMPLE;
    Serial.println("testing CAN bus speed polling");
    while (true)
    {
        canbus_poll();
        obd_queryTask(now = millis());
        obdstat_reportTask(now);
    }
}

void bringuptest_heartbeat()
{
    heartbeat_init();
    while (true)
    {
        heartbeat_task(millis());
    }
}

void bringuptest_stripAnimation()
{
    strip_init();
    hud_animation = HUDANI_FADEIN;
    hud_aniStep = 0;
    while (true)
    {
        stripe_animate_step();
        vTaskDelay(5 + hud_aniDelay);
    }
}

void bringuptest_speedcalibration()
{
    static uint32_t last_time = 0;
    uint32_t now;
    dbg_ser.enabled = true;
    heartbeat_init();
    canbus_init();
    battlog_init();
    battlog_startNewLog();
    speedcalib_log = true;
    speedcalib_active = true;
    obd_poll_mode = OBDPOLLMODE_SIMPLE;
    while (true)
    {
        canbus_poll();
        obd_queryTask(now = millis());
        speedcalib_task(now);
        car_data.speed_mph = speedcalib_convert(spdpredict_get(now));
        battlog_task(now);
        heartbeat_task(now);
        obdstat_reportTask(now);
        esp_task_wdt_reset();
        vTaskDelay(5);
    }
}