#include "HeadUpDisplay.h"

extern bool obd_debug_dump;
extern uint32_t obd_debug_rate;
extern uint8_t obd_poll_mode;
extern bool obd_hasNewLog;
extern bool obd_hasNewSpeed;
extern bool obd_isResponding;
extern bool speedcalib_log;
extern bool speedcalib_active;
extern int32_t amblight_val;

void bringup_tests()
{
    //bringuptest_ambientlight();
    //bringuptest_adc();
    //bringuptest_time();
    //bringuptest_sdcard();
    //bringuptest_heartbeat();
    //bringuptest_canbusquery();
    //bringuptest_canbusspy();
    //bringuptest_canbusspeed();
    bringuptest_speedlog();
    //bringuptest_stripAnimation();
}

void bringuptest_console()
{
    while (true)
    {
        cmdline.task();
    }
}

void bringuptest_time()
{
    uint32_t last_time = 0;
    while (true)
    {
        cmdline.task();
        uint32_t now = millis();
        if ((now - last_time) >= 1000)
        {
            last_time = now;
            Serial.printf("%u, ", now);
            time_print(&Serial, now);
            Serial.println();
        }
    }
}

void bringuptest_ambientlight()
{
    uint32_t last_time = 0;
    amblight_init();
    while (true)
    {
        amblight_task();
        uint32_t now = millis();
        if ((now - last_time) >= 1000)
        {
            last_time = now;
            //Serial.printf("%u, %d, %d\r\n", now, amblight_val, amblight_get());
        }
    }
}

void bringuptest_adc()
{
    uint32_t last_time = 0;
    while (true)
    {
        uint32_t now = millis();
        if ((now - last_time) >= 1000)
        {
            last_time = now;
            Serial.printf("%u, %d, %d\r\n", now, analogRead(HUD_PIN_AMBLIGHT));
        }
    }
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
    obd_poll_mode = OBDPOLLMODE_BATTLOG_LONG;
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
            Serial.println();
            obd_hasNewLog = false;
            obd_hasNewSpeed = false;
        }
        //obdstat_reportTask(now);
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
    uint8_t repeat_animation = HUDANI_VOLTMETER_FADEOUT;
    car_data.aux_batt_volt_x10 = 120;
    hud_animation = 0;
    hud_aniStep = 0;
    while (true)
    {
        if (repeat_animation != hud_animation)
        {
            hud_animation = repeat_animation;
            hud_aniStep = 0;
            Serial.println("animation start");
            if (repeat_animation >= HUDANI_FADEIN && repeat_animation < HUDANI_FADEOUT)
            {
                strip_blank();
            }
            else if (repeat_animation >= HUDANI_FADEOUT && repeat_animation <= HUDANI_FADEOUT_END)
            {
                strip_speedometer(0);
            }
        }
        stripe_animate_step();
        vTaskDelay(LOOP2_MIN_DELAY + MS_TO_RTOS_TICKS(hud_aniDelay));
    }
}

void bringuptest_speedcalibration()
{
    static uint32_t last_time = 0;
    static bool start_logging = false;
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
        car_data.speed_mph = speedcalib_convert(car_data.rpm);
        #if 0
        if (obd_isResponding != false && start_logging == false) {
            battlog_startNewLog();
            start_logging = true;
        }
        if (start_logging != false)
        {
            battlog_task(now);
        }
        #else
        battlog_task(now);
        #endif
        heartbeat_task(now);
        obdstat_reportTask(now);
        esp_task_wdt_reset();
        vTaskDelay(5);
    }
}

void bringuptest_speedlog()
{
    static uint32_t last_time = 0;
    static bool start_logging = false;
    uint32_t now;
    dbg_ser.enabled = true;
    heartbeat_init();
    canbus_init();
    battlog_init();
    battlog_startNewLog();
    speedcalib_log = false;
    speedcalib_active = false;
    obd_poll_mode = OBDPOLLMODE_SIMPLE;
    while (true)
    {
        canbus_poll();
        obd_queryTask(now = millis());
        speedcalib_task(now);
        car_data.rpm_guess = spdpredict_get(millis());
        car_data.speed_mph = speedcalib_convert(car_data.rpm_guess);
        #if 0
        if (obd_isResponding != false && start_logging == false) {
            battlog_startNewLog();
            start_logging = true;
        }
        if (start_logging != false)
        {
            battlog_task(now);
        }
        #else
        battlog_task(now);
        #endif
        heartbeat_task(now);
        obdstat_reportTask(now);
        esp_task_wdt_reset();
        vTaskDelay(5);
    }
}

