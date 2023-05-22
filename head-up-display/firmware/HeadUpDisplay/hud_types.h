#ifndef _HUD_TYPES_H_
#define _HUD_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

#include "hud_config.h"
#include "hud_defs.h"

typedef struct
{
    int16_t  rpm;
    int16_t  rpm_guess;
    int16_t  rpm_max;
    int16_t  speed_kmh;
    int16_t  speed_kmh_max;
    float    speed_mph;     // contains MPH results after conversion and prediction
    uint8_t  throttle;
    uint8_t  gear;
    bool     brake;
    uint8_t  turn_sig;

    uint16_t aux_batt_volt_x10;
    int32_t  batt_current_x10;
    uint32_t batt_voltage_x10;
    int32_t  batt_power_x100;
    uint8_t  bms_soc_x2;
    uint16_t bms_soh_x10;
    uint8_t  soc_disp_x2;
    int32_t  req_current_x10;

    uint8_t  batt_fan_feedback;
    uint8_t  batt_fan_status;
    int16_t  temperature_indoor_x2;
    int16_t  temperature_outdoor_x2;
    int16_t  temperature_batt_heater;

    bool     ignition;
    bool     contactor;
    uint8_t  charge_mode;
    uint32_t operating_time_sec;

    uint32_t cumulative_charge_current;
    uint32_t cumulative_discharge_current;
    uint32_t cumulative_charge_power;
    uint32_t cumulative_discharge_power;

    uint8_t  cellvolt_min_x50;
    uint8_t  cellvolt_max_x50;
    uint16_t cellhealth_min_x10;
    bool     cellvolt_recommend_balancing;

    uint16_t inverter_capacitor_voltage;
    uint16_t isolation_resistance;

    uint32_t idle_time_ms;
}
vehicle_data_t;

typedef struct
{
    uint32_t magic;
    uint32_t len;

    int32_t  speed_multiplier; // actual variable used to calculate MPH from RPM, denominator is fixed
    #if ENABLE_SPEED_CALIBRATION
    uint16_t speed_kmh_max;    // data used for calibration
    uint16_t speed_calib_rpm;  // data used for calibration
    uint16_t speed_calib_kmh;  // data used for calibration
    #endif

    uint32_t amblight_low;    // sensor reading while darkest
    uint32_t amblight_high;   // sensor reading while brightest
    uint32_t amblight_min;    // minimum global LED brightness (during darkness)
    uint32_t amblight_expo;   // curve of sensor response
    uint32_t amblight_filter; // LPF filter constant, out of 1000

    float spdpredict_slew;      // limit on speed change due to prediction (0 = prediction is off)
    float spdpredict_factor;     // how dominant the prediction is in the result (0 = prediction is off)
    float spdpredict_jerkFilter; // LPF filter constant for the jerk (0 = filter is off)

    // LED brightness for animation elements
    uint8_t ledbrite_tick;
    uint8_t ledbrite_bar;
    uint8_t ledbrite_volt;

    float cell_imbalance; // cell imbalance voltage threshold

    uint32_t crc32;
}
settings_t;

typedef struct
{
    uint32_t pid;
    uint32_t dev;
}
query_t;

#endif
