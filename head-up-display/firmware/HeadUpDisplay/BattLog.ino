#include "HeadUpDisplay.h"

#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include <MultiDestPrinter.h>

bool battlog_cardReady = false;
bool battlog_fileReady = false;
uint8_t battlog_writeErrCnt;

char battlog_filename[64] = { 0 };

static File battlog_filePtr;

extern MultiDestPrinter log_printer;

void battlog_init()
{
    battlog_cardReady = false;
    battlog_fileReady = false;

    if(!SD.begin(HUD_PIN_SD_CS)){
        Serial.println("ERROR: SD card init failed");
        return;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("ERROR: SD card type is \"NONE\"");
        return;
    }

    Serial.print("SD card type: ");
    switch (cardType)
    {
        case CARD_MMC:  Serial.println("MMC");     break;
        case CARD_SD:   Serial.println("SD");      break;
        case CARD_SDHC: Serial.println("SDHC");    break;
        default:        Serial.println("UNKNOWN"); break;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD card size: %lluMB\r\n", cardSize);

    battlog_cardReady = true;

    battlog_nameGen();
}

void battlog_nameGen()
{
    if (battlog_cardReady == false) {
        return;
    }

    File root = SD.open("/");
    if (!root) {
        Serial.println("ERROR: Failed to open root directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("ERROR: root is not a directory");
        return;
    }

    int max_fname_num = 0;
    char* fname = battlog_filename;

    File file_iter = root.openNextFile();
    while (file_iter)
    {
        strncpy(fname, file_iter.name(), 32);
        if (memcmp(fname, "battlog_", 8) == 0)
        {
            char* num_start = (char*)&(fname[8]);
            for (int i = 0; i < strlen(num_start); i++)
            {
                if (num_start[i] == '.') {
                    num_start[i] = 0;
                    break;
                }
            }
            int suffix_num = atoi(num_start);
            if (suffix_num > max_fname_num) {
                max_fname_num = suffix_num;
            }
        }
        file_iter = root.openNextFile();
    }

    max_fname_num += 1;
    max_fname_num %= 100000;

    sprintf(battlog_filename, "/battlog_%u.csv", max_fname_num);
}

void battlog_startNewLog()
{
    if (battlog_filename == NULL || battlog_filename[0] == 0) {
        battlog_nameGen();
    }
    if (battlog_filename == NULL || battlog_filename[0] == 0) {
        return;
    }

    battlog_filePtr = SD.open(battlog_filename, FILE_APPEND);
    if (!battlog_filePtr)
    {
        Serial.printf("ERROR: failed to open file %s for writing\r\n", battlog_filename);
    }
    else
    {
        Serial.printf("Opened file %s for logging\r\n", battlog_filename);
        battlog_fileReady = true;
        battlog_writeErrCnt = 0;
    }
}

void battlog_close()
{
    if (battlog_fileReady == false) {
        return;
    }
    battlog_filePtr.close();
    battlog_fileReady = false;
    Serial.println("Closed logging file");
}

void battlog_log()
{
    if (battlog_fileReady == false) {
        return;
    }
    Print* p1 = dynamic_cast<Print*>(&battlog_filePtr);
    log_printer.destinations[LOGPRINTER_IDX_SDCARD] = p1;
    if (p1 == NULL) {
        Serial.printf("ERROR: battlog_filePtr failed to cast into Print*\r\n");
    }
    Print* p2 = dynamic_cast<Print*>(&log_printer);
    if (p1 == NULL) {
        Serial.printf("ERROR: log_printer failed to cast into Print*\r\n");
    }

    volatile uint32_t start_time = millis(); // for time measurement, checks for card timeout event, getWriteError on the file pointer does not work

    obd_printLog(p2);
    p2->printf("\r\n");
    battlog_filePtr.flush();

    volatile uint32_t end_time = millis();
    volatile uint32_t dtime = end_time - start_time;
    if (dtime >= 200) // checks for card timeout event, getWriteError on the file pointer does not work
    {
        battlog_writeErrCnt++;
        battlog_filePtr.clearWriteError();
        if ((car_data.ignition == false && battlog_writeErrCnt > 1) || (car_data.ignition != false && battlog_writeErrCnt >= 1)) {
            battlog_fileReady = false;
            battlog_cardReady = false;
            Serial.printf("[%u]: SD write error\r\n", millis());
            if (car_data.ignition == false) {
                // TODO: reinitialize?
            }
        }
    }
    else
    {
        battlog_writeErrCnt = 0;
    }
}

void battlog_task(uint32_t now)
{
    if (battlog_fileReady == false) {
        return;
    }

    static uint32_t last_log_time = 0;
    //static uint32_t last_flush_time = 0;

    uint32_t tick_interval = 1000;

    if (car_data.ignition != false) {
        tick_interval = 200;
    }
    else if (car_data.charge_mode != 0) {
        tick_interval = 500;
    }

    if ((now - last_log_time) >= tick_interval)
    {
        if ((now - last_log_time) >= tick_interval * 2) {
            last_log_time = now;
        }
        else {
            last_log_time += tick_interval;
        }
        battlog_log();
    }
}