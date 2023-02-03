#include "HeadUpDisplay.h"
#include <EEPROM.h>

extern uint32_t crc32_le(uint32_t crc, uint8_t const *buf, uint32_t len);
extern uint32_t fletcher32(const uint16_t *data, size_t len);

//#define CHECKSUM(_buf, _len) crc32_le(0, (uint8_t*)(_buf), (size_t)(_len))
#define CHECKSUM(_buf, _len) fletcher32((const uint16_t*)(_buf), (size_t)(_len))

void settings_default()
{
    memset((void*)&hud_settings, 0, sizeof(settings_t));

    hud_settings.len = sizeof(settings_t);

    // set the default setting values here
    //hud_settings.speed_multiplier = 0;
    hud_settings.speed_multiplier = RPM2MPH_E6;
    #ifdef ENABLE_SPEED_CALIBRATION
    hud_settings.speed_kmh_max = 0;
    hud_settings.speed_calib_rpm = 0;
    hud_settings.speed_calib_kmh = 0;
    #endif

    hud_settings.spdpredict_slew = 35;
    hud_settings.spdpredict_factor = 0.8;

    hud_settings.ledbrite_tick = 0xFF;
    hud_settings.ledbrite_bar  = 0xFF;
    hud_settings.ledbrite_volt = 0x80;

    hud_settings.amblight_filter = 20;
    hud_settings.amblight_min = 64;
    hud_settings.amblight_high = 0;
    hud_settings.amblight_low = (1024 * 4) - 1;
}

bool settings_load() {
  EEPROM.begin(sizeof(settings_t));
  //dbg_ser.printf("settings struct size = %u\r\n", sizeof(settings_t));
  EEPROM.readBytes(0, (void*)(&hud_settings), sizeof(settings_t));
  uint32_t crc_calced = CHECKSUM(&hud_settings, sizeof(settings_t) - sizeof(uint32_t));
  // if the data is invalid, report so
  if (hud_settings.magic != SETTINGS_MAGIC || crc_calced != hud_settings.crc32) {
    Serial.printf("settings not valid, magic 0x%08X, CRC 0x%08X ?= 0x%08X\r\n", hud_settings.magic, crc_calced, hud_settings.crc32);
    return false;
  }
  return true;
}

void settings_init() {
  if (settings_load() == false) { // if the data is invalid, load default values
    settings_default();
  }
}

void settings_save() {
  // save the data with a valid checksum so it can be loaded next time
  hud_settings.magic = SETTINGS_MAGIC;
  hud_settings.len = sizeof(settings_t);

  uint32_t crc_calced = CHECKSUM(&hud_settings, sizeof(settings_t) - sizeof(uint32_t));
  hud_settings.crc32 = crc_calced;
  EEPROM.writeBytes(0, (const void*)(&hud_settings), sizeof(settings_t));
  EEPROM.commit();
}

uint32_t settings_saveLaterTime = 0;
uint32_t settings_saveLaterTimeFirst = 0;

void settings_saveLater()
{
    uint32_t now = millis();
    // delay the flash writing to preserve flash life
    settings_saveLaterTime = now;
    if (settings_saveLaterTimeFirst <= 0) {
        settings_saveLaterTimeFirst = now;
    }
}

void settings_saveTask(uint32_t now, bool force)
{
    if (settings_saveLaterTime != 0 && (((now - settings_saveLaterTime) >= 1000 || (now - settings_saveLaterTimeFirst) >= 5000) || force)) {
        settings_save();
        settings_saveLaterTime = 0;
        settings_saveLaterTimeFirst = 0;
    }
}

void settings_factoryReset()
{
    settings_default();
    settings_saveTask(millis(), true);
}

void settings_report(Print* p)
{
    if (p == NULL) {
        return;
    }
    p->print("settings,");
    p->printf("%d,"   , hud_settings.speed_multiplier);
    p->printf("%d,"   , hud_settings.amblight_low);
    p->printf("%d,"   , hud_settings.amblight_high);
    p->printf("%d,"   , hud_settings.amblight_min);
    p->printf("%d,"   , hud_settings.amblight_expo);
    p->printf("%d,"   , hud_settings.amblight_filter);
    p->printf("%.03f,", hud_settings.spdpredict_slew);
    p->printf("%.03f,", hud_settings.spdpredict_factor);
    p->printf("%.03f,", hud_settings.spdpredict_jerkFilter);
    p->printf("%d,"   , hud_settings.ledbrite_tick);
    p->printf("%d,"   , hud_settings.ledbrite_bar);
    p->printf("%d,"   , hud_settings.ledbrite_volt);
    p->printf("%.03f,", hud_settings.cell_imbalance);
    p->print("\r\n");
}

uint32_t fletcher32(const uint16_t *data, size_t len)
{
    // https://en.wikipedia.org/wiki/Fletcher%27s_checksum
    uint32_t c0, c1;
    len = (len + 1) & ~1;      /* Round up len to words */

    /* We similarly solve for n > 0 and n * (n+1) / 2 * (2^16-1) < (2^32-1) here. */
    /* On modern computers, using a 64-bit c0/c1 could allow a group size of 23726746. */
    for (c0 = c1 = 0; len > 0; ) {
        size_t blocklen = len;
        if (blocklen > 360*2) {
            blocklen = 360*2;
        }
        len -= blocklen;
        do {
            c0 = c0 + *data++;
            c1 = c1 + c0;
        } while ((blocklen -= 2));
        c0 = c0 % 65535;
        c1 = c1 % 65535;
    }
    return (c1 << 16 | c0);
}
