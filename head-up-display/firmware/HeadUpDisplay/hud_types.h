#ifndef _HUD_TYPES_H_
#define _HUD_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

#include "hud_config.h"
#include "hud_defs.h"

typedef struct
{
    int16_t  rpm;
    int16_t  rpm_max;
    int16_t  speed_kmh;
    int16_t  speed_kmh_max;
    float    speed_mph;
    uint8_t  throttle;

    uint16_t aux_batt_volt_x10;
    int16_t  batt_current_x10;
    uint16_t batt_voltage_x10;
    int16_t  batt_power_x100;
    uint8_t  bms_soc_x2;
    uint16_t bms_soh_x10;
    uint8_t  soc_disp_x2;

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

    uint16_t inverter_capacitor_voltage;
    uint16_t isolation_resistance;
}
vehicle_data_t;

typedef struct
{
    uint32_t magic;
    uint32_t len;

    uint32_t speed_multiplier;
    uint16_t speed_kmh_max;
    uint16_t speed_calib_rpm;
    uint16_t speed_calib_kmh;

    uint32_t amblight_low;
    uint32_t amblight_min;
    uint32_t amblight_expo;
    uint32_t amblight_high;

    float spdpredict_slew;
    float spdpredict_accel;
    float spdpredict_jerk;

    uint32_t crc32;
}
settings_t;

#endif
