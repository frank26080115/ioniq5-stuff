#ifndef _HUD_DEFS_H_
#define _HUD_DEFS_H_

#include <stdint.h>
#include <stdbool.h>

#include "hud_config.h"

#define SETTINGS_MAGIC 0xBEEF4321

enum
{
    OBDPOLLMODE_IDLE,
    //OBDPOLLMODE_EXTRAFAST,
    OBDPOLLMODE_SIMPLE,
    OBDPOLLMODE_BATTLOG_SHORT,
    OBDPOLLMODE_BATTLOG_LONG,
};

enum
{
    HUDSTATE_INIT,
    HUDSTATE_IDLECHECK,
    HUDSTATE_IGNITION,
    HUDSTATE_MOVING,
    HUDSTATE_OFF,
};

enum
{
    HUDANI_OFF,
    HUDANI_INTRO,
    HUDANI_SPEEDOMETER,
    HUDANI_FADEIN,
    HUDANI_FADEIN_ALL,
    HUDANI_FADEIN_MELT,
    HUDANI_FADEIN_SCROLL,
    HUDANI_FADEIN_SCROLL_OUTSIDEIN,
    HUDANI_FADEIN_SCROLL_OUTSIDEIN_FADE,
    HUDANI_FADEIN_FADESCROLL,
    HUDANI_FADEIN_FADESCROLL_INSIDEOUT,
    HUDANI_FADEIN_FADESCROLL_OUTSIDEIN,
    HUDANI_FADEIN_SCROLLFADE,
    HUDANI_FADEIN_RANDOM,
    HUDANI_FADEIN_RANDOM_2,
    HUDANI_FADEIN_STATIC,
    HUDANI_FADEIN_STATIC_2,
    HUDANI_FADEIN_FUZZ,
    HUDANI_FADEIN_FUZZ_2,
    HUDANI_FADEIN_LAST,
    HUDANI_FADEOUT,
    HUDANI_FADEOUT_ALL,
    HUDANI_FADEOUT_RANDOM,
    HUDANI_FADEOUT_RANDOM_2,
    HUDANI_FADEOUT_RANDOM_3,
    HUDANI_FADEOUT_SEQ,
    HUDANI_FADEOUT_SEQ_OUTSIDEIN,
    HUDANI_FADEOUT_SEQ_INSIDEOUT,
    HUDANI_FADEOUT_SCROLL,
    HUDANI_FADEOUT_SCROLLFADE,
    HUDANI_FADEOUT_SCROLL_INSIDEOUT,
    HUDANI_FADEOUT_SCROLL_INSIDEOUT_FADE,
    HUDANI_FADEOUT_END,
    HUDANI_VOLTMETER,
    HUDANI_VOLTMETER_FADEIN,
    HUDANI_VOLTMETER_FADEOUT,
};

#define MS_TO_RTOS_TICKS(x) ((x) / portTICK_RATE_MS)

#define OBD_PID_CELLVOLT_0  0x0102 // battery cell voltages
#define OBD_PID_CELLVOLT_1  0x0103 // battery cell voltages
#define OBD_PID_CELLVOLT_2  0x0104 // battery cell voltages
#define OBD_PID_CELLVOLT_3  0x010A // battery cell voltages
#define OBD_PID_CELLVOLT_4  0x010B // battery cell voltages
#define OBD_PID_CELLVOLT_5  0x010C // battery cell voltages
#define OBD_PID_REALSPEED   0x0100 // real vehicle speed, indoor temperature, outdoor temperature
#define OBD_PID_MAINPACKET  0x0101 // main data packet
#define OBD_PID_BATT2NDARY  0x0105 // upper bank cell temperatures, state of health
#define OBD_PID_ODOMETER    0xB002
#define OBD_PID_THROTTLE    0x49 // 0x11
#define OBD_PID_SIMPLESPEED 0x0D

#define OBD_PACKET_START (2 + 3)

//#define RPM2MPH_E6    12000 // 12000 is calculated with a 4.71 gear ratio
#define RPM2MPH_E6     7851 // calculated from the log
#define KMH2MPH_E6   621371
#define KMH2MPH_DIV 1000000

#define LOGPRINTER_IDX_SDCARD  0
#define LOGPRINTER_IDX_WEBSOCK 1

#define CRGB_PURPLE(x)      CRGB((uint8_t)((x) - 8), (uint8_t)0, (uint8_t)((x) - 8))
#define CRGB_ORANGE(x, d)   CRGB((uint8_t)(x), (uint8_t)(((x) + ((d) / 2)) / (d)), (uint8_t)0)
#define CRGB_RED(x)         CRGB((uint8_t)(x), (uint8_t)0, (uint8_t)0)
#define CRGB_BLUE(x)        CRGB((uint8_t)0, (uint8_t)0, (uint8_t)(x))
#define CRGB_WHITE(x)       CRGB((uint8_t)(x), (uint8_t)(x), (uint8_t)(x))
#define CRGB_BLACK()        CRGB((uint8_t)0, (uint8_t)0, (uint8_t)(0))

#define GET_LED(x)          leds[LED_STRIP_OFFSET + (x)]
#define SET_LED(x, y)       GET_LED(x) = (y)

#endif
