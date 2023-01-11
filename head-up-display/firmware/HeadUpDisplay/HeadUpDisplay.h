#ifndef _HEADUPDISPLAY_H_
#define _HEADUPDISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

#include "hud_config.h"
#include "hud_defs.h"
#include "hud_types.h"

#include <Arduino.h>

#include <DebuggingSerial.h>
#include <SerialCmdLine.h>
#include <WebSocketPrinter.h>
#include <MultiDestPrinter.h>

extern DebuggingSerial dbg_ser;
extern MultiDestPrinter log_printer;

extern vehicle_data_t car_data;
extern settings_t hud_settings;

extern bool speedcalib_active;

extern int32_t amblight_val;

#endif
