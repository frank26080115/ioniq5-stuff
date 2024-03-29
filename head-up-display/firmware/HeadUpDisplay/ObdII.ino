#include "HeadUpDisplay.h"

uint8_t* obd_database[32] = { NULL };
bool     obd_queryPending  = false;
uint32_t obd_lastQueryTime = 0;
uint32_t obd_lastRespTime  = 0;
//uint32_t obd_firstTimeMs = 0;
//uint32_t obd_firstTimeUs = 0;
uint8_t  obd_poll_mode = OBDPOLLMODE_SIMPLE;
bool     obd_debug_dump = false;
uint32_t obd_debug_rate = 1;
bool     obd_hasNewLog = false;
bool     obd_hasNewSpeed = false;
bool     obd_gotNewSpeed = false;
bool     obd_isResponding = false;

query_t obd_queryQueueFifo[32];
uint8_t obd_queryQueueW = 0;
uint8_t obd_queryQueueR = 0;

int32_t obdstat_rxCnt = 0;
int32_t obdstat_txCnt = 0;
int32_t obdstat_logCnt = 0;

extern bool speedcalib_log;

void obd_queryTask(uint32_t tnow)
{
    static uint8_t tick = 0;

    uint32_t qrate = 51; // value of 51 here got about 10 updates per second, unable to push it any faster
    uint32_t qtimeout = 200;

    if (car_data.ignition == false)
    {
        qrate = 200;
        qtimeout = 600;
    }

    if ((tnow - obd_lastRespTime) >= qtimeout) {
        obd_isResponding = false;
    }

    if ((obd_queryPending == false && (tnow - obd_lastQueryTime) >= qrate * obd_debug_rate) || (obd_queryPending != false && (tnow - obd_lastQueryTime) >= qtimeout))
    {
        if (obd_poll_mode != OBDPOLLMODE_IDLE)
        {
            if (tick == 0)
            {
                canbus_queryEnhancedPid(OBD_PID_MAINPACKET, 0x7E4);

                obd_queryQueueW = 0;
                obd_queryQueueR = 0;

                tick = 1;

                //obd_queryQueuePush(OBD_PID_INDICATORS, 0x770);

                if (car_data.speed_mph < 5)
                {
                    obd_queryQueuePush(OBD_PID_THRGEAR, 0x7E4);
                    obd_queryQueuePush(OBD_PID_BRAKE, 0x7A0);
                }

                if (hud_settings.speed_multiplier == 0 || speedcalib_active)
                {
                    obd_queryQueuePush(OBD_PID_REALSPEED, 0x7B3);
                }

                if (obd_poll_mode == OBDPOLLMODE_BATTLOG_SHORT)
                {
                    obd_queryQueuePush(OBD_PID_BATT2NDARY, 0x7E4);
                }
                else if (obd_poll_mode == OBDPOLLMODE_BATTLOG_LONG)
                {
                    obd_queryQueuePush(OBD_PID_BATT2NDARY, 0x7E4);
                    obd_queryQueuePush(OBD_PID_CELLVOLT_0, 0x7E4);
                    obd_queryQueuePush(OBD_PID_CELLVOLT_1, 0x7E4);
                    obd_queryQueuePush(OBD_PID_CELLVOLT_2, 0x7E4);
                    obd_queryQueuePush(OBD_PID_CELLVOLT_3, 0x7E4);
                    obd_queryQueuePush(OBD_PID_CELLVOLT_4, 0x7E4);
                    obd_queryQueuePush(OBD_PID_CELLVOLT_5, 0x7E4);
                }
            }
            else if (tick == obd_queryQueueW / 2 && obd_queryQueueW >= 4)
            {
                // get the RPM reading frequently
                canbus_queryEnhancedPid(OBD_PID_MAINPACKET, 0x7E4);
                tick += 1;
            }
            else
            {
                obd_queryQueuePop();
                if (obd_queryQueueR == obd_queryQueueW) // queue empty
                {
                    tick = 0; // restart the queue on next iteration
                }
            }

            obd_queryPending = true;
            obd_lastQueryTime = tnow;
            obd_lastRespTime = tnow;
        }
    }
}

bool obd_parse(uint8_t* data, uint16_t datalen)
{
    uint32_t now = millis();

    if (obd_debug_dump != false)
    {
        dbg_ser.printf("CAN RX[s=%u, t=%u]: ", datalen, millis());
        int di;
        for (di = 0; di < datalen; di++)
        {
            dbg_ser.printf("0x%02X, ", data[di]);
        }
        dbg_ser.printf("\r\n");
        /*
        CAN RX[t=29143]: 0x22, 0x01, 0x01, 
        CAN RX[t=29231]: 0x62, 0x01, 0x01, 0xEF, 0xFB, 0xE7, 0xEF, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1B, 0xB7, 0x10, 0x0E, 0x0E, 0x0F, 0x0E, 0x0F, 0x0E, 0x00, 0x35, 0xB8, 0x02, 0xB8, 0x0D, 0x00, 0x00, 0x87, 0x00, 0x00, 0x1E, 0xE2, 0x00, 0x00, 0x1E, 0xB1, 0x00, 0x00, 0x16, 0x80, 0x00, 0x00, 0x15, 0xAD, 0x00, 0x13, 0xE0, 0x40, 0x00, 0x02, 0xC4, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB8, 
        
        CAN RX[t=29231]: 0x62, 0x01, 0x01, 0xEF, 0xFB, 0xE7, 0xEF, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1B, 0xB7, 0x10, 0x0E, 0x0E, 0x0F, 0x0E, 0x0F, 0x0E, 0x00, 0x35, 0xB8, 0x02, 0xB8, 0x0D, 0x00
                     0 1 2     3     4     5     6     7     8     9
                                           A     B     C     D     E
        */

    }

    int16_t pid8 = -1;
    uint16_t pid16 = 0;
    uint8_t service_code = data[0];
    if (service_code == 0x62) // response to 0x22
    {
        obd_queryPending = false;
        pid16 = data[1];
        pid16 <<= 8;
        pid16 |= data[2];

        pid8 = pid16 & 0xFF;
        if (data[1] != 0x01) {
            pid8 |= 0x10;
        }
    }
    else if (service_code == 0x41 || service_code == 0x42)
    {
        pid8 = (data[1] & 0x0F) | 0x10;
    }

    if (pid8 >= 0 && pid8 <= 0x1F)
    {
        uint16_t* ptr16 = (uint16_t*)(obd_database[pid8]);
        if (obd_database[pid8] == NULL)
        {
            obd_database[pid8] = (uint8_t*)malloc(datalen + 2);
            ptr16 = (uint16_t*)(obd_database[pid8]);
            if (obd_debug_dump)
            {
                dbg_ser.printf("CAN bus parser created DB 0x%02X -> %u\r\n", pid8, datalen);
            }
        }
        else if ((*ptr16) < datalen)
        {
            free(obd_database[pid8]);
            obd_database[pid8] = (uint8_t*)malloc(datalen + 2);
            if (obd_debug_dump)
            {
                dbg_ser.printf("CAN bus parser resized DB 0x%02X -> %u\r\n", pid8, datalen);
            }
        }

        if (obd_database[pid8] != NULL)
        {
            obd_lastRespTime = now;
            obd_isResponding = true;
            obdstat_rxCnt++;
            *ptr16 = datalen;
            memcpy((void*)(&(ptr16[1])), (const void*)data, (size_t)datalen);

            switch (pid8)
            {
                case (OBD_PID_SIMPLESPEED & 0x0F) | 0x10:
                    obd_hasNewSpeed = true;
                    obd_parseVehicleDataSpeedOnly();
                    if (hud_settings.speed_multiplier <= 0 || speedcalib_active) {
                        obd_parseVehicleDataSpeedCalibration();
                    }
                    spdpredict_submit(car_data.rpm, now);
                    break;
                case (OBD_PID_THROTTLE & 0x0F) | 0x10:
                case (OBD_PID_THRGEAR  & 0x0F) | 0x10:
                case (OBD_PID_BRAKE    & 0x0F) | 0x10:
                    obd_parseVehicleDataThrottle();
                    break;
                case (OBD_PID_MAINPACKET & 0x1F):
                    obd_hasNewSpeed = true; // indicate to user app
                    obd_gotNewSpeed = true; // indicate to this module
                    obd_parseVehicleDataBasic();
                    spdpredict_submit(car_data.rpm, now);
                    break;
                case (OBD_PID_REALSPEED & 0x1F):
                    obd_hasNewLog = true;
                    if (obd_gotNewSpeed == false)
                    {
                        obd_parseVehicleDataBasic();
                        spdpredict_submit(car_data.rpm, now);
                    }
                    obd_gotNewSpeed = false;
                    if (hud_settings.speed_multiplier <= 0 || speedcalib_active) {
                        obd_parseVehicleDataSpeedCalibration();
                    }
                    obd_parseVehicleDataBattery();
                    break;
                case (OBD_PID_INDICATORS  & 0x0F) | 0x10:
                    obd_parseVehicleDataIndicators();
                    break;
            }

            return true;
        }
    }

    return false;
}

void obd_printCellVoltages(Print* p)
{
    if (p == NULL) {
        return;
    }

    p->print("cellvolts:,");

    char tmpstr[16];
    uint8_t pids[] = {
        OBD_PID_CELLVOLT_0 & 0xFF,
        OBD_PID_CELLVOLT_1 & 0xFF,
        OBD_PID_CELLVOLT_2 & 0xFF,
        OBD_PID_CELLVOLT_3 & 0xFF,
        OBD_PID_CELLVOLT_4 & 0xFF,
        OBD_PID_CELLVOLT_5 & 0xFF,
    };
    int i, j;
    for (i = 0; i <= 5; i++)
    {
        uint8_t* db = obd_database[pids[i]];
        if (db == NULL)
        {
            for (j = 0; j < 32; j++)
            {
                p->print("0, ");
            }
        }
        else
        {
            for (j = 0; j < 32; j++)
            {
                float x = db[OBD_PACKET_START + j];
                x /= 50.0f;
                sprintf(tmpstr, "%0.2f, ", x);
                p->print((const char *)tmpstr);
            }
        }
    }
}

void obd_printBattBankTemperature_internal(Print* p, uint16_t pid, char idx_letter, uint8_t cnt)
{
    if (p == NULL) {
        return;
    }

    char tmpstr[16];
    uint8_t* db;
    int i, j;
    db = obd_database[pid & 0xFF];
    if (db != NULL)
    {
        j = (int)(idx_letter - 'A');
        for (i = 0; i < cnt; i++)
        {
            int8_t x = (int8_t)(db[OBD_PACKET_START + j + i]);
            sprintf(tmpstr, "%d, ", x);
            p->print((const char *)tmpstr);
        }
    }
    else
    {
        for (i = 0; i < cnt; i++)
        {
            p->print("-128, ");
        }
    }
}

void obd_printBattBankTemperatures(Print* p)
{
    if (p != NULL) {
        p->print("banktemps:,");
    }
    obd_printBattBankTemperature_internal(p, OBD_PID_MAINPACKET, 'Q', 5);
    obd_printBattBankTemperature_internal(p, OBD_PID_BATT2NDARY, 'J', 10);
}

void obd_parseVehicleDataSpeedOnly()
{
    if (obd_database[(OBD_PID_SIMPLESPEED & 0x0F) | 0x10] != NULL)
    {
        uint8_t* ptr = (uint8_t*)(&((obd_database[(OBD_PID_SIMPLESPEED & 0x0F) | 0x10])[2]));
        car_data.speed_kmh = ptr[1];
        car_data.speed_kmh_max = car_data.speed_kmh > car_data.speed_kmh_max ? car_data.speed_kmh : car_data.speed_kmh_max;
    }
    else
    {
        obd_parseVehicleDataBasic();
    }
}

void obd_parseVehicleDataThrottle()
{
    if (obd_database[(OBD_PID_THROTTLE & 0x0F) | 0x10] != NULL)
    {
        // WARNING: car never replied to this request, the parsing has not been tested
        uint8_t* ptr = (uint8_t*)(&((obd_database[(OBD_PID_THROTTLE & 0x0F) | 0x10])[2]));
        car_data.throttle = ptr[1];
    }
    if (obd_database[(OBD_PID_THRGEAR & 0x0F) | 0x10] != NULL)
    {
        uint8_t* ptr = (uint8_t*)(&((obd_database[(OBD_PID_THRGEAR & 0x0F) | 0x10])[OBD_PACKET_START]));
        car_data.throttle =  ptr['J' - 'A'] / 2;
        car_data.gear     = (ptr['O' - 'A'] & 0x0F) + 1;
    }
    if (obd_database[(OBD_PID_BRAKE & 0x0F) | 0x10] != NULL)
    {
        uint8_t* ptr = (uint8_t*)(&((obd_database[(OBD_PID_BRAKE & 0x0F) | 0x10])[OBD_PACKET_START]));
        car_data.brake = (ptr['G' - 'A'] & 0x10) != 0;
    }
}

void obd_parseVehicleDataBasic()
{
    if (obd_database[OBD_PID_MAINPACKET & 0xFF] == NULL) {
        return;
    }

    static int8_t assume_ignition = 0;
    static uint32_t stop_time = 0;

    uint8_t* ptr = (uint8_t*)(&((obd_database[OBD_PID_MAINPACKET & 0xFF])[OBD_PACKET_START]));

    int16_t rpm1 = pktparse_sint16_be(ptr, 53);
    int16_t rpm2 = pktparse_sint16_be(ptr, 53 + 2);
    rpm1 *= (rpm1 < 0) ? -1 : 1;
    rpm2 *= (rpm2 < 0) ? -1 : 1;

    car_data.rpm = rpm1 >= rpm2 ? rpm1 : rpm2;
    if (car_data.rpm > car_data.rpm_max) {
        car_data.rpm_max = car_data.rpm;
    }

    if (car_data.rpm != 0)
    {
        stop_time = 0;
        car_data.idle_time_ms = 0;
    }
    else if (car_data.rpm == 0 && stop_time <= 0)
    {
        stop_time = millis();
    }
    else if (car_data.rpm == 0 && stop_time > 0)
    {
        car_data.idle_time_ms = millis() - stop_time;
    }

    car_data.batt_current_x10  = pktparse_sint16_be(ptr, 'K' - 'A');
    car_data.batt_voltage_x10  = pktparse_uint16_be(ptr, 'M' - 'A');
    car_data.batt_power_x100   = car_data.batt_current_x10 * car_data.batt_voltage_x10;
    car_data.aux_batt_volt_x10 = ptr[29];

    car_data.ignition = ((ptr[50] & (1 << 2)) != 0);
                        // || ((ptr['J' - 'A'] & (1 << 0)) != 0);

    if (car_data.ignition != false) {
        assume_ignition = -1;
    }

    if (assume_ignition == 0 && car_data.ignition == false && (car_data.rpm != 0 || car_data.gear > GEAR_PARK))
    {
        assume_ignition = 1;
        car_data.ignition = true;
    }

    if (assume_ignition == 1)
    {
        car_data.ignition = true;
    }

    if (car_data.rpm == 0 && car_data.gear <= GEAR_PARK && car_data.idle_time_ms >= 3000 && (car_data.batt_current_x10 < -50 || car_data.charge_mode != 0))
    {
        assume_ignition = 0;
        car_data.ignition = false;
    }

    car_data.cellvolt_min_x50   = ptr['Z' - 'A'];
    car_data.cellvolt_max_x50   = ptr['X' - 'A'];
    car_data.cellhealth_min_x10 = pktparse_uint16_be(ptr, 28);
    // unable to find data item for cellhealth_max

    // determine if balancing is recommended, TODO: tune this
    if (car_data.cellvolt_max_x50 < 4 * 50)
    {
        float vdiff = car_data.cellvolt_max_x50 - car_data.cellvolt_min_x50;
        vdiff /= 50;
        car_data.cellvolt_recommend_balancing |= vdiff > hud_settings.cell_imbalance;
    }

    obd_parseVehicleDataThrottle();
}

void obd_parseVehicleDataSpeedCalibration()
{
    uint8_t* ptr;
    if (obd_database[OBD_PID_REALSPEED & 0xFF] != NULL)
    {
        ptr = (uint8_t*)(&((obd_database[OBD_PID_REALSPEED & 0xFF])[OBD_PACKET_START]));
        car_data.speed_kmh = ptr[29];
    }
    else if (obd_database[(OBD_PID_SIMPLESPEED & 0x0F) | 0x10] != NULL)
    {
        // WARNING: car never replied to this request, the parsing has not been tested
        ptr = (uint8_t*)(&((obd_database[(OBD_PID_SIMPLESPEED & 0x0F) | 0x10])[2]));
        car_data.speed_kmh = ptr[1];
    }

    car_data.speed_kmh_max = car_data.speed_kmh > car_data.speed_kmh_max ? car_data.speed_kmh : car_data.speed_kmh_max;
}

void obd_parseVehicleDataIndicators()
{
    uint8_t* ptr;
    if (obd_database[(OBD_PID_INDICATORS & 0x0F) | 0x10] != NULL)
    {
        ptr = (uint8_t*)(&((obd_database[(OBD_PID_INDICATORS & 0x0F) | 0x10])[OBD_PACKET_START]));
        car_data.turn_sig = ptr[7];
    }
}

void obd_parseVehicleDataBattery()
{
    if (obd_database[OBD_PID_MAINPACKET & 0xFF] == NULL || obd_database[OBD_PID_BATT2NDARY & 0xFF] == NULL || obd_database[OBD_PID_REALSPEED & 0xFF] == NULL) {
        return;
    }

    uint8_t* ptr1 = (uint8_t*)(&((obd_database[OBD_PID_MAINPACKET & 0xFF])[OBD_PACKET_START]));
    uint8_t* ptr2 = (uint8_t*)(&((obd_database[OBD_PID_BATT2NDARY & 0xFF])[OBD_PACKET_START]));
    uint8_t* ptr3 = (uint8_t*)(&((obd_database[OBD_PID_REALSPEED  & 0xFF])[OBD_PACKET_START]));

    car_data.aux_batt_volt_x10 = ptr1[29];
    car_data.bms_soc_x2 = ptr1['E' - 'A'];
    car_data.soc_disp_x2 = ptr2[31];
    car_data.bms_soh_x10 = pktparse_uint16_be(ptr2, 'Z' - 'A');
    car_data.req_current_x10 = pktparse_uint16_be(ptr1, 'H' - 'A');

    car_data.charge_mode = ptr1['J' - 'A'];

    car_data.batt_fan_feedback        = ptr1[28];
    car_data.batt_fan_status          = ptr1[27];
    car_data.temperature_indoor_x2    = ptr3['F' - 'A'];
    car_data.temperature_outdoor_x2   = ptr3['G' - 'A'];
    car_data.temperature_batt_heater  = ptr2['X' - 'A'];

    car_data.cumulative_charge_current    = pktparse_uint32_be(ptr1, 30);
    car_data.cumulative_discharge_current = pktparse_uint32_be(ptr1, 30 + 4);
    car_data.cumulative_charge_power      = pktparse_uint32_be(ptr1, 30 + 8);
    car_data.cumulative_discharge_power   = pktparse_uint32_be(ptr1, 30 + 12);

    car_data.inverter_capacitor_voltage = pktparse_uint16_be(ptr1, 51);
    car_data.isolation_resistance       = pktparse_uint16_be(ptr1, 57);
}

void obd_printLog(Print* p)
{
    #ifdef ENABLE_TEST_PRINT_DB
    obd_printDb(p);
    return;
    #endif
    float tmp;

    if (p == NULL) {
        return;
    }

    time_print(p, millis());
    tmp = millis();
    tmp /= 1000.0;
    p->printf("%0.1f, ", tmp);
    obdstat_logCnt++;

    p->printf("%d, %0.1f, %d, ", car_data.rpm, car_data.speed_mph, car_data.speed_kmh);

    #ifdef ENABLE_TEST_PRINT_SPEED_FAST
    p->printf("%d, ", car_data.rpm_guess);
    return;
    #endif

    #if ENABLE_SPEED_CALIBRATION
    if (speedcalib_log != false) {
        p->printf("%u, %u, %u, %u, ", hud_settings.speed_multiplier, hud_settings.speed_kmh_max, hud_settings.speed_calib_rpm, hud_settings.speed_calib_kmh);
    }
    #endif

    p->printf("%u, 0x%02X, ", car_data.ignition, car_data.charge_mode);

    p->printf("%u, 0x%02X, %u, ", car_data.throttle, car_data.gear, car_data.brake);

    tmp = car_data.batt_current_x10;
    tmp /= 10.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.batt_voltage_x10;
    tmp /= 10.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.batt_power_x100;
    tmp /= 100.0;
    p->printf("%0.1f, ", tmp);

    p->printf("%u, ", car_data.inverter_capacitor_voltage);

    tmp = car_data.bms_soc_x2;
    tmp /= 2.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.soc_disp_x2;
    tmp /= 2.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.bms_soh_x10;
    tmp /= 10.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.aux_batt_volt_x10;
    tmp /= 10.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.req_current_x10;
    tmp /= 10.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.cumulative_charge_current;
    tmp /= 10.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.cumulative_discharge_current;
    tmp /= 10.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.cumulative_charge_power;
    tmp /= 10.0;
    p->printf("%0.1f, ", tmp);

    tmp = car_data.cumulative_discharge_power;
    tmp /= 10.0;
    p->printf("%0.1f, ", tmp);

    p->printf("%u, ", car_data.batt_fan_feedback);
    p->printf("%u, ", car_data.batt_fan_status);
    tmp = car_data.temperature_indoor_x2;
    tmp /= 2.0;
    tmp -= 40.0;
    p->printf("%0.1f, ", tmp);
    tmp = car_data.temperature_outdoor_x2;
    tmp /= 2.0;
    tmp -= 40.0;
    p->printf("%0.1f, ", tmp);
    p->printf("%u, ", car_data.temperature_batt_heater);

    p->printf("%u, ", car_data.isolation_resistance);

    obd_printCellVoltages(p);
    obd_printBattBankTemperatures(p);
}

void obd_printDb(Print* p)
{
    float tmp;

    if (p == NULL) {
        return;
    }

    uint32_t now = millis();

    int i;

    time_print(p, now);
    tmp = now;
    tmp /= 1000.0;
    p->printf("%0.1f, ", tmp);
    p->printf(";;;;;");

    for (i = 0; i < 32; i++)
    {
        if (obd_database[i] == NULL) {
            continue;
        }
        int16_t* dblen_ptr = (int16_t*)(obd_database[i]);
        int16_t dblen = *dblen_ptr;
        p->printf("%u, %d, ", i, dblen);
        int j;
        for (j = 0; j < dblen; j++)
        {
            p->printf("0x%02X, ", obd_database[i][2 + j]);
        }
        p->printf(";;;;;");
    }
}

void obdstat_reportTask(uint32_t now)
{
    static uint32_t last_time = 0;
    
    if ((now - last_time) >= 1000)
    {
        last_time = now;
        Serial.printf("[%u]: OBD2-STAT: %u ; %u ; %u\r\n", now, obdstat_rxCnt, obdstat_txCnt, obdstat_logCnt);
    }
}

void obd_queryQueuePush(uint32_t pid, uint32_t dev)
{
    obd_queryQueueFifo[obd_queryQueueW].pid = pid;
    obd_queryQueueFifo[obd_queryQueueW].dev = dev;
    obd_queryQueueW += 1;
}

void obd_queryQueuePop()
{
    uint32_t pid, dev;
    pid = obd_queryQueueFifo[obd_queryQueueR].pid;
    dev = obd_queryQueueFifo[obd_queryQueueR].dev;
    obd_queryQueueR += 1;
    canbus_queryEnhancedPid(pid, dev);
}
