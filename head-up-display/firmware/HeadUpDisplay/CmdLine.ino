#include "HeadUpDisplay.h"
#include <SerialCmdLine.h>

#if 0
void factory_reset_func(void* cmd, char* argstr, Stream* stream);
void echo_func         (void* cmd, char* argstr, Stream* stream);
void memcheck_func     (void* cmd, char* argstr, Stream* stream);
void dumpsettings_func (void* cmd, char* argstr, Stream* stream);
void reboot_func       (void* cmd, char* argstr, Stream* stream);
void debug_func        (void* cmd, char* argstr, Stream* stream);
void listlog_func      (void* cmd, char* argstr, Stream* stream);
void query8_func       (void* cmd, char* argstr, Stream* stream);
void query16_func      (void* cmd, char* argstr, Stream* stream);
void speedcalib_func   (void* cmd, char* argstr, Stream* stream);
void settime_func      (void* cmd, char* argstr, Stream* stream);
#endif


const cmd_def_t cmds[] = {
  { "factoryreset", factory_reset_func},
  { "echo"        , echo_func },
  { "mem"         , memcheck_func },
  { "dumpsettings", dumpsettings_func },
  { "reboot"      , reboot_func },
  { "debug"       , debug_func },
  { "listlog"     , listlog_func },
  { "speedcalib"  , speedcalib_func },
  { "query8"      , query8_func },
  { "query16"     , query16_func },
  { "settime"     , settime_func },
  { "logstop"     , logstop_func },
  { "logstart"    , logstart_func },
  { "", NULL }, // end of table
};

SerialCmdLine cmdline(&Serial, (cmd_def_t*)cmds, false, (char*)">>>", (char*)"???", true, 512);

void factory_reset_func(void* cmd, char* argstr, Stream* stream)
{
    settings_default();
    settings_save();
    stream->println("factory reset performed");
}

void echo_func(void* cmd, char* argstr, Stream* stream)
{
    stream->println(argstr);
}

void reboot_func(void* cmd, char* argstr, Stream* stream)
{
    stream->println("rebooting...\r\n\r\n");
    ESP.restart();
}

void memcheck_func(void* cmd, char* argstr, Stream* stream)
{
    stream->printf("free heap mem: %u\r\n", ESP.getFreeHeap());
}

void dumpsettings_func(void* cmd, char* argstr, Stream* stream)
{
    int i;
    uint8_t* ptr = (uint8_t*)&hud_settings;
    stream->printf("Dumping settings data struct:");
    for (i = 0; i < sizeof(settings_t); i++)
    {
        if ((i % 16) == 0)
        {
            stream->printf("\r\n");
        }
        stream->printf("0x%02X ", ptr[i]);
    }
    stream->printf("\r\n");
}

void debug_func(void* cmd, char* argstr, Stream* stream)
{
    dbg_ser.enabled = atoi(argstr) != 0;
    stream->printf("debugging output = %u\r\n", dbg_ser.enabled);
    dbg_ser.println("test output from debugging serial port");
}

void listlog_func(void* cmd, char* argstr, Stream* stream)
{
    File root = SD.open("/");
    File file = root.openNextFile();
    stream->println("listing log files");
    while(file)
    {
        if (memcmp(file.name(), "battlog_", 8) == 0)
        {
            stream->print("file: ");
            stream->println(file.name());
        }
        file = root.openNextFile();
    }
    stream->println("end of file listing");
}

void query8_func(void* cmd, char* argstr, Stream* stream)
{
    char* pos2;
    unsigned long pid = strtoul(argstr, &pos2, 16);
    unsigned long adr = strtoul(pos2, NULL, 16);
    stream->printf("CAN bus manual query8 0x%02X 0x%04X\r\n", pid, adr);
    canbus_queryStandardPid(pid, adr);
}

void query16_func(void* cmd, char* argstr, Stream* stream)
{
    char* pos2;
    unsigned long pid = strtoul(argstr, &pos2, 16);
    unsigned long adr = strtoul(pos2, NULL, 16);
    stream->printf("CAN bus manual query16 0x%04X 0x%04X\r\n", pid, adr);
    canbus_queryEnhancedPid(pid, adr);
}

void speedcalib_func(void* cmd, char* argstr, Stream* stream)
{
    int x = atoi(argstr);
    stream->printf("speed calibration %s\r\n", x == 0 ? "OFF" : "ON");
    speedcalib_active = x != 0;
}

void amblightread_func(void* cmd, char* argstr, Stream* stream)
{
    int x = atoi(argstr);
    stream->printf("amblient light debug %s\r\n", x == 0 ? "OFF" : "ON");
    amblight_dump = x != 0;
}

void settime_func(void* cmd, char* argstr, Stream* stream)
{
    time_set_epoch(atol(argstr));
    stream->print("time set, current: ");
    Print* p = dynamic_cast<Print*>(stream);
    time_print(p, millis());
    stream->print("\r\n");
}

void logstop_func(void* cmd, char* argstr, Stream* stream)
{
    battlog_close();
    stream->print("logging stopped\r\n");
}

void logstart_func(void* cmd, char* argstr, Stream* stream)
{
    battlog_startNewLog();
    stream->print("logging stopped\r\n");
}
