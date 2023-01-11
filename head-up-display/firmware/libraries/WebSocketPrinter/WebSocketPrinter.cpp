#include <WebSocketPrinter.h>

WebSocketPrinter::WebSocketPrinter(AsyncWebSocket* s)
{
    this->ws_obj = s;
}

size_t WebSocketPrinter::write(uint8_t c)
{
    if (this->enabled) {
        cache[cache_idx] = c;
        cache_idx++;
        if (cache_idx >= WSPRINTER_CACHE_SIZE || c == '\n' || c == '\0') {
            write(NULL, 0);
            cache_idx = 0;
        }
    }
    #if ARDUINO >= 100
    return 1;
    #endif
}

size_t WebSocketPrinter::write(const uint8_t *buffer, size_t size)
{
    if (this->enabled) {
        if (buffer == NULL && size == 0) {
            ((AsyncWebSocket*)this->ws_obj)->binaryAll((const char*)this->cache, (size_t)this->cache_idx);
            return size;
        }
        else {
            if (this->cache_idx > 0) {
                ((AsyncWebSocket*)this->ws_obj)->binaryAll((const char*)this->cache, (size_t)this->cache_idx);
                this->cache_idx = 0;
            }
            ((AsyncWebSocket*)this->ws_obj)->binaryAll((const char*)buffer, (size_t)size);
            return size;
        }
    }
    return 0;
}
