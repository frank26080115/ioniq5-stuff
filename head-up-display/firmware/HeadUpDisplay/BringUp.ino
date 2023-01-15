#include "HeadUpDisplay.h"

extern bool obd_debug_dump;
extern uint32_t obd_debug_rate;
extern uint8_t obd_poll_mode;
extern bool obd_hasNewLog;
extern bool obd_hasNewSpeed;

void bringup_tests()
{
    //bringuptest_heartbeat();
    //bringuptest_canbusquery();
    //bringuptest_canbusspy();
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
    canbus_init();
    obd_debug_dump = true;
    obd_debug_rate = 10;
    obd_poll_mode = OBDPOLLMODE_BATTLOG_LONG;
    Serial.println("testing CAN bus polling");
    while (true)
    {
        canbus_poll();
        obd_queryTask(millis());
        if (obd_hasNewLog)
        {
            obd_printLog(dynamic_cast<Print*>(&Serial));
            obd_hasNewLog = false;
        }
    }
}

void bringuptest_canbusspeed()
{
    canbus_init();
    obd_debug_dump = true;
    obd_debug_rate = 10;
    //obd_poll_mode = OBDPOLLMODE_EXTRAFAST;
    Serial.println("testing CAN bus speed polling");
    while (true)
    {
        canbus_poll();
        obd_queryTask(millis());
        if (obd_hasNewSpeed)
        {
            obd_printLog(dynamic_cast<Print*>(&Serial));
            obd_hasNewSpeed = false;
        }
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