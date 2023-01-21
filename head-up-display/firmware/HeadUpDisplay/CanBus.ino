#include "driver/gpio.h"
#include "driver/can.h"

#define CANBUS_TX_ID     0x7E4 // 0x7DF

#define ISOTP_BUFSIZE_RX 256
#define ISOTP_BUFSIZE_TX 32

IsoTpLink isotp_link;
uint8_t canbus_bufferRx[ISOTP_BUFSIZE_RX];
uint8_t canbus_bufferTx[ISOTP_BUFSIZE_TX];
uint8_t canbus_payloadRx[ISOTP_BUFSIZE_RX];
uint8_t canbus_payloadTx[ISOTP_BUFSIZE_TX];

extern int32_t obdstat_txCnt;

void canbus_init()
{
    // Initialize configuration structures using macro initializers
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(HUD_PIN_CANBUS_TX, HUD_PIN_CANBUS_RX, CAN_MODE_NORMAL);
    can_timing_config_t  t_config = CAN_TIMING_CONFIG_500KBITS();
    can_filter_config_t  f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    //Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        if (can_start() == ESP_OK) {
            dbg_ser.println("CAN bus driver installed and started");
        }
        else {
            Serial.println("CAN bus driver installed but failed to start");
        }
    } else {
        Serial.println("CAN bus driver failed to install");
    }

    isotp_init_link(&isotp_link, CANBUS_TX_ID, canbus_bufferTx, ISOTP_BUFSIZE_TX, canbus_bufferRx, ISOTP_BUFSIZE_RX);
}

void isotp_user_debug(const char* message, ...)
{
    #if 0
    char loc_buf[64];
    char * temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, message);
    va_copy(copy, arg);
    int len = vsnprintf(temp, sizeof(loc_buf), message, copy);
    va_end(copy);
    if (len < 0) {
        va_end(arg);
        return;
    }
    if(len >= (int)sizeof(loc_buf)) {
        temp = (char*) malloc(len + 1);
        if(temp == NULL) {
            va_end(arg);
            return;
        }
        len = vsnprintf(temp, len + 1, message, arg);
    }
    va_end(arg);
    len = Serial.write((uint8_t*)temp, len);
    if (temp != loc_buf) {
        free(temp);
    }
    #endif
    return;
}

int isotp_user_send_can(const uint32_t arbitration_id, const uint8_t* data, const uint8_t size)
{
    can_message_t message;
    message.identifier = arbitration_id;
    message.flags = CAN_MSG_FLAG_SS;
    message.data_length_code = size;
    for (int i = 0; i < size; i++) {
        message.data[i] = data[i];
    }
    esp_err_t ret;
    if ((ret = can_transmit(&message, pdMS_TO_TICKS(1000))) == ESP_OK) {
        return ISOTP_RET_OK;
    } else {
        if (ret == ESP_ERR_TIMEOUT) {
            return ISOTP_RET_TIMEOUT;
        }
        else if (ret == ESP_FAIL) {
            return ISOTP_RET_INPROGRESS;
        }
        else {
            return ISOTP_RET_ERROR;
        }
    }
}

uint32_t isotp_user_get_ms(void)
{
    return (uint32_t)millis();
}

bool canbus_poll()
{
    esp_err_t ret1; int ret2;
    can_message_t message;
    if ((ret1 = can_receive(&message, pdMS_TO_TICKS(1))) == ESP_OK)
    {
        if ((message.flags & CAN_MSG_FLAG_RTR) == 0) {
            isotp_on_can_message(&isotp_link, message.data, message.data_length_code);
        }
    }
    isotp_poll(&isotp_link);
    uint16_t out_size;
    if ((ret2 = isotp_receive(&isotp_link, canbus_payloadRx, ISOTP_BUFSIZE_RX, &out_size)) == ISOTP_RET_OK)
    {
        return obd_parse(canbus_payloadRx, out_size);
    }
    return false;
}

void canbus_queryEnhancedPid(uint16_t pid, uint16_t devid)
{
    uint8_t* ptr = (uint8_t*)(&pid);
    canbus_payloadTx[0] = 0x22; // enhanced request
    canbus_payloadTx[1] = ptr[1];
    canbus_payloadTx[2] = ptr[0];

    isotp_link.send_arbitration_id = devid; // if this isn't set here, the flow control messages will use the old ID
    // technically the correct way of doing this is to initialize two different link objects, but that wastes a lot of buffer

    isotp_send_with_id(&isotp_link, devid, canbus_payloadTx, 3);

    obdstat_txCnt++;
}

void canbus_queryStandardPid(uint8_t pid, uint16_t devid)
{
    // WARNING: this function does not work with the IONIQ 5, the computer doesn't respond to PIDs usually used for ICE
    canbus_payloadTx[0] = 0x01; // standard request
    canbus_payloadTx[1] = pid;
    isotp_link.send_arbitration_id = devid;
    isotp_send_with_id(&isotp_link, devid, canbus_payloadTx, 2);
}
