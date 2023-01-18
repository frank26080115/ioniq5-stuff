#ifndef _HUD_CONFIG_H_
#define _HUD_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

#define HUD_PIN_CANBUS_RX  GPIO_NUM_25
#define HUD_PIN_CANBUS_TX  GPIO_NUM_26

#define HUD_PIN_AMBLIGHT   GPIO_NUM_23

#define HUD_PIN_SD_CS      GPIO_NUM_5

#define HUD_PIN_STRIP_DATA GPIO_NUM_15
#define HUD_PIN_STRIP_CLK  GPIO_NUM_27

#define HUD_PIN_HEART_DATA GPIO_NUM_2
#define HUD_PIN_HEART_CLK  GPIO_NUM_12
#define HUD_PIN_HEART_PWR  GPIO_NUM_13

#define LED_STRIP_SIZE       ((8 * 6) + 1)
#define SPEED_TICK_SPACING   6
#define VOLT_TICK_SPACING    9
#define SPEED_BACKFADE       8
#define SPEED_NEEDLE_SIZE    2
#define SPEED_MAX            120
#define FADING_HEAD

#endif
