int16_t pktparse_sint16_be(uint8_t* buf, int idx)
{
    uint16_t x = pktparse_uint16_be(buf, idx);
    int16_t* ptr = (int16_t*)(&x);
    return *ptr;
}

uint16_t pktparse_uint16_be(uint8_t* buf, int idx)
{
    uint16_t x;
    x = buf[idx];
    x <<= 8;
    x |= buf[idx + 1];
    return x;
}

int32_t pktparse_sint32_be(uint8_t* buf, int idx)
{
    uint32_t x = pktparse_uint32_be(buf, idx);
    int32_t* ptr = (int32_t*)(&x);
    return *ptr;
}

uint32_t pktparse_uint32_be(uint8_t* buf, int idx)
{
    uint32_t x;
    x = buf[idx];
    x <<= 8;
    x |= buf[idx + 1];
    x <<= 8;
    x |= buf[idx + 2];
    x <<= 8;
    x |= buf[idx + 3];
    return x;
}

int32_t pktparse_sint24_be(uint8_t* buf, int idx)
{
    uint32_t x;
    x = buf[idx];
    x <<= 8;
    x |= buf[idx + 1];
    x <<= 8;
    x |= buf[idx + 2];
    x <<= 8;
    int32_t* ptr = (int32_t*)(&x);
    *ptr /= 256;
    return *ptr;
}

uint32_t pktparse_uint24_be(uint8_t* buf, int idx)
{
    uint32_t x;
    x = buf[idx];
    x <<= 8;
    x |= buf[idx + 1];
    x <<= 8;
    x |= buf[idx + 2];
    return x;
}

void lpf_update(int32_t n, int32_t* pm, int32_t x, int32_t m)
{
    if ((*pm) < 0)
    {
        *pm = n * m;
        return;
    }
    int32_t prev = *pm;
    int32_t old_sub = prev / x;
    int32_t old_val = prev - old_sub;
    int32_t new_val = n * m;
    new_val /= x;
    new_val += old_val;
    *pm = new_val;
}

int32_t lpf_read(int32_t pm, int32_t m)
{
    pm += m / 2;
    return pm / m;
}

int str_endswith(const char *str, const char *suffix)
{
    if (!str || !suffix) {
        return 0;
    }
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr) {
        return 0;
    }
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

uint32_t time_epoch = 0;

void time_set_epoch(uint32_t x)
{
    uint32_t ms = millis() / 1000;
    time_epoch = x - ms;
}

void time_print(Print* p, uint32_t ms)
{
    time_t rawtime = time_epoch + (ms / 1000);
    struct tm *infop;
    struct tm info;
    infop = localtime(&rawtime);
    memcpy(&info, infop, sizeof(struct tm));
    switch (info.tm_mon)
    {
        case 0:  p->print("JAN");  break;
        case 1:  p->print("FEB");  break;
        case 2:  p->print("MAR");  break;
        case 3:  p->print("APR");  break;
        case 4:  p->print("MAY");  break;
        case 5:  p->print("JUN");  break;
        case 6:  p->print("JULY"); break;
        case 7:  p->print("AUG");  break;
        case 8:  p->print("SEPT"); break;
        case 9:  p->print("OCT");  break;
        case 10: p->print("NOV");  break;
        case 11: p->print("DEC");  break;
    }
    p->printf("-%u", info.tm_mday);
    p->printf("-%u", info.tm_year + 1900);
    p->printf(" %u:%02u:%02u.%u ,", info.tm_hour, info.tm_min, info.tm_sec, (ms % 1000) / 100);
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double mapd(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double expo_curve(double x, double curve)
{
    double ax = x < 0 ? -x : x;
    double ac = curve < 0 ? -curve : curve;
    double etop = ac * ( ax - 1 );
    double eres = exp(etop);
    double y = ax * eres;
    if (curve < 0)
    {
        y = 1 - y;
    }
    if (x < 0)
    {
        y = -y;
    }
    return y;
}

void srand_check()
{
    static bool has_srand = false;
    if (has_srand) {
        return;
    }
    uint32_t seed;
    srand(seed = esp_random());
    has_srand = true;
    dbg_ser.printf("[%u]: SRAND seeded 0x%08X\r\n", millis(), seed);
}
