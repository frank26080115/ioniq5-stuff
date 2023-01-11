#include <MultiDestPrinter.h>

MultiDestPrinter::MultiDestPrinter()
{
}

size_t MultiDestPrinter::write(uint8_t c)
{
    cache[cache_idx] = c;
    cache_idx++;
    if (cache_idx >= MDESTPRINTER_CACHE_SIZE || c == '\n' || c == '\0') {
        write_to_all(cache, cache_idx);
        cache_idx = 0;
    }
    #if ARDUINO >= 100
    return 1;
    #endif
}

size_t MultiDestPrinter::write(const uint8_t *buffer, size_t size)
{
    if (cache_idx > 0) {
        write_to_all(cache, cache_idx);
        cache_idx = 0;
    }
    return write_to_all(buffer, size);
}

size_t MultiDestPrinter::write_to_all(const uint8_t *buffer, size_t size)
{
    uint8_t i;
    for (i = 0; i < MDESTPRINTER_DEST_CNT; i++)
    {
        Print* p = destinations[i];
        if (p != NULL)
        {
            p->write(buffer, size);
        }
    }
    return size;
}
