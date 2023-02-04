#ifndef _WEBSOCKETPRINTER_H_
#define _WEBSOCKETPRINTER_H_

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#define WSPRINTER_CACHE_SIZE (1024 * 4)

class WebSocketPrinter : public Print
{
    public:
        WebSocketPrinter(AsyncWebSocket* ws);
        bool enabled;
        size_t write(uint8_t c);
        size_t write(const uint8_t *buffer, size_t size);
    protected:
        AsyncWebSocket* ws_obj;
        uint8_t cache[WSPRINTER_CACHE_SIZE];
        uint16_t cache_idx;
};

#endif
