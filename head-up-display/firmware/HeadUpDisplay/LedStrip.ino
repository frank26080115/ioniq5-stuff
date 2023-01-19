#include "HeadUpDisplay.h"

extern void hsv2rgb_rainbow(const CHSV &hsv, CRGB &rgb);

CRGB leds[LED_STRIP_SIZE];
CLEDController* strip_ctrler;

void strip_init()
{
    strip_ctrler = &FastLED.addLeds<DOTSTAR, HUD_PIN_STRIP_DATA, HUD_PIN_STRIP_CLK, BGR>(leds, LED_STRIP_SIZE);
}

void strip_task(uint32_t now)
{
}

void strip_speedometer(double speed)
{
    int8_t tickspace;
    int i;
    double spd = speed;
    double spdboost;
    int32_t baridx;
    #ifdef FADING_HEAD
    float headbright;
    int32_t headbrighti;
    #endif
    int trailsize;
    int tickspeed = 10;
    int spdmax1, spdmax2;
    int addbar;

    spdmax1 = LED_STRIP_SIZE / SPEED_TICK_SPACING;
    spdmax1 *= tickspeed; // result should be 80
    spdmax2 = SPEED_MAX; // limit beyond the last tick

    // there's a offset between the readings here and my car's real speedometer
    // I wanted to shift the needle another LED up
    // this is a good way to do it without messing with the "addbar" variable
    spdboost = ((double)spdmax1)/((double)LED_STRIP_SIZE);
    spdboost += -0.5; // calibrate this on the bench
    if (spdboost < 0.0) {
        spdboost = 0.0;
    }

    tickspace = SPEED_TICK_SPACING;
    trailsize = SPEED_NEEDLE_SIZE;
    trailsize += (int)lround(spd - ((double)spdmax1)); // sorry this is a bit lazy
    if (trailsize < SPEED_NEEDLE_SIZE) {
        trailsize = SPEED_NEEDLE_SIZE;
    }

    if (speed < 10.0)
    {
        // I don't want to apply the offset too early
        // mainly because I still want the needle hidden at zero
        spdboost *= speed;
        spdboost /= 10.0;
    }

    spd += spdboost;
    speed += spdboost;

    spd *= LED_STRIP_SIZE;
    spd /= spdmax1;
    #ifndef FADING_HEAD
    baridx = (int32_t)lround(spd);
    #else
    baridx = (int32_t)floor(spd);
    #endif

    // for better UX, center of "needle" is the speed
    addbar = SPEED_NEEDLE_SIZE;
    addbar /= 2;
    baridx += addbar;

    // if baridx is negative, we don't care

    #ifdef FADING_HEAD
    if (speed < ((float)spdmax1))
    {
        headbright  = (speed * LED_STRIP_SIZE);
        headbright  = fmodf(headbright, ((float)spdmax1));
        headbright *= hud_settings.ledbrite_bar;
        headbright /= ((float)spdmax1);
        headbrighti = (int)lroundf(headbright);
    }
    else if (speed >= ((float)spdmax1))
    {
        float rmd = speed - floor(speed);
        headbright = rmd * hud_settings.ledbrite_bar;
        headbrighti = (int)lroundf(headbright);
        // since the bar grows from right to left in this case,
        // the "head" (on the right) is hidden by the final tick
        // what we actually care about is the dimming of the "tail" (on the left)
    }
    #endif

    if (baridx >= (LED_STRIP_SIZE - 1)) {
        baridx = (LED_STRIP_SIZE - 1);
    }

    for (i = 0; i < LED_STRIP_SIZE; i++)
    {
        if (i == 0 && speed < 0.5)
        {
            leds[i] = CRGB_BLACK(); // this will be overwritten by the first tick
        }
        else if (i <= baridx && i >= (baridx - trailsize)) // is part of the needle
        {
            #ifdef FADING_HEAD
            if (i == baridx)
            {
                leds[i] = CRGB_ORANGE(headbrighti, 4);
            }
            else if (i == (baridx - trailsize))
            {
                leds[i] = CRGB_ORANGE(hud_settings.ledbrite_bar - headbrighti, 4);
            }
            else
            #endif
            {
                leds[i] = CRGB_ORANGE(hud_settings.ledbrite_bar, 4);
            }
        }
        else
        {
            leds[i] = CRGB_BLACK(); // default to nothing
        }

        if ((i % tickspace) == 0) // is a tick
        {
            if (i <= baridx)
            {
                if (i == 0) {
                    leds[i] = CRGB_BLUE(hud_settings.ledbrite_tick);
                }
                else if ((i / tickspace) <= 4) {
                    leds[i] = CRGB_RED(hud_settings.ledbrite_tick);
                }
                else {
                    leds[i] = CRGB_PURPLE(hud_settings.ledbrite_tick);
                }
            }
            else {
                leds[i] = CRGB_BLUE(hud_settings.ledbrite_tick);
            }
        }
    }

    if (leds[0].r == 0 && leds[0].g == 0 && leds[0].b == 0) {
        leds[0] = CRGB_BLUE(hud_settings.ledbrite_tick);
    }
}

void strip_voltmeter(float v, uint8_t b)
{
    int tick_space = 12;
    float y = mapf(v, 11.5, 13, 0, VOLT_TICK_SPACING * 5);
    int yi = lround(y);
    int i;
    for (i = 0; i < LED_STRIP_SIZE; i++)
    {
        if ((i % tick_space) == 0)
        {
            if (yi >= LED_STRIP_SIZE) {
                leds[i] = CRGB_PURPLE(b);
            }
            else {
                leds[i] = CRGB_BLUE(b);
            }
        }
        else if (i <= yi)
        {
            int h = lround(mapf(i, 0, LED_STRIP_SIZE - (LED_STRIP_SIZE / 4), HUE_RED, HUE_GREEN));
            h = h > HUE_GREEN ? HUE_GREEN : h;
            CHSV hsv = CHSV(h, 255, b);
            hsv2rgb_rainbow(hsv, leds[i]);
        }
        else
        {
            leds[i] = CRGB_BLACK();
        }
    }
}

void strip_indicateRegen()
{
    static uint32_t last_time = 0;
    static uint32_t tick_speed = 100;
    static int tick = -1;
    static bool was_regen = false;
    uint32_t now = millis();

    // TODO: check if actually in regenerative braking
    bool is_regen;

    if (is_regen)
    {
        // TODO: set ticking speed according to regen level

        int i;
        int j = lround(floor(car_data.speed_mph / 10.0));
        int k1 = j - 2;
        int k4 = j + 1;
        while (k1 < 0) {
            k1++;
            k4++;
        }
        while (k4 > 8)
        {
            k1--;
            k4--;
        }

        if (was_regen == false || tick < 0)
        {
            last_time = now;
            tick = k4;
        }
        else if ((now - last_time) >= tick_speed)
        {
            tick--;
            if (tick < k1)
            {
                tick = k4;
            }
        }
        i = tick * SPEED_TICK_SPACING;
        leds[i].r /= 2;
        leds[i].g /= 2;
        leds[i].b /= 2;

        was_regen = true;
    }
    else
    {
        was_regen = false;
        tick = -1;
    }
}

void stripe_animate_step()
{
    bool need_show = false;
    switch (hud_animation)
    {
        case HUDANI_OFF:
            hud_aniDelay = 100;
            break;
        case HUDANI_SPEEDOMETER:
            strip_speedometer(car_data.speed_mph);
            strip_indicateRegen();
            hud_aniDelay = 10;
            need_show = true;
            break;
        case HUDANI_INTRO:
            {
                if (hud_aniStep == 0) {
                    dbg_ser.printf("[%u]: ANI intro start\r\n", millis());
                }

                bool did = false;
                int i, j, s, stage, step, size;
                int mididx1, mididx2;
                step = hud_aniStep;
                size = 2;
                mididx1 = LED_STRIP_SIZE;
                mididx1 /= 2;
                mididx1 -= 1;
                mididx2 = mididx1 + (((LED_STRIP_SIZE % 2) == 0) ? 1 : 0);

                stage = step + 1;
                s = (stage < size) ? stage : size;

                for (i = 0; i < LED_STRIP_SIZE; i++) {
                    leds[i] = CRGB_BLACK();
                }
                for (i = mididx1 - step, j = 0; i >= 0 && j < s; i--, j++)
                {
                    leds[i] = CRGB_WHITE(0xFF);
                    did = true;
                }
                for (i = mididx2 + step, j = 0; i < LED_STRIP_SIZE && j < s; i++, j++)
                {
                    leds[i] = CRGB_WHITE(0xFF);
                    did = true;
                }
                if (did == false)
                {
                    hud_animation = hud_animation_queue;
                    hud_animation_queue = HUDANI_OFF;
                    hud_aniStep = 0;
                    hud_aniDelay = 1;
                }
                else
                {
                    hud_aniStep += 1;
                    hud_aniDelay = 50;
                    need_show = true;
                }
            }
            break;
        case HUDANI_FADEIN:
        case HUDANI_FADEOUT:
            {
                dbg_ser.printf("[%u]: ANI fade %u\r\n", millis(), hud_animation);
                int num_of_ani = ((hud_animation == HUDANI_FADEIN) ? (HUDANI_FADEIN_LAST - HUDANI_FADEIN) : (HUDANI_FADEOUT_END - HUDANI_FADEOUT)) - 1;
                srand_check();
                hud_animation += 1 + (rand() % num_of_ani);
                hud_aniStep = 0;
            }
            hud_aniDelay = 0;
            break;
        case HUDANI_FADEOUT_END:
            strip_blank();
            hud_animation = hud_animation_queue;
            hud_animation_queue = HUDANI_OFF;
            hud_aniStep = 0;
            hud_aniDelay = 100;
            need_show = true;
            dbg_ser.printf("[%u]: ANI fade out end -> %u\r\n", millis(), hud_animation);
            break;
        case HUDANI_FADEIN_ALL:
        case HUDANI_FADEIN_MELT:
            {
                strip_aniFadeInReport();
                hud_aniDelay = 20;
                need_show = true;
                int b = hud_aniStep * 4;
                b = b >= hud_settings.ledbrite_tick ? hud_settings.ledbrite_tick : b;
                int meltlimit = hud_settings.ledbrite_tick / 2;
                int b2 = (hud_animation == HUDANI_FADEIN_ALL) ? 0 : ((b <= (meltlimit)) ? b :(meltlimit - (b - meltlimit) - 1));
                b2 = b2 < 0 ? 0 : b2;
                int i;
                for (i = 0; i < LED_STRIP_SIZE; i++)
                {
                    leds[i] = CRGB_BLUE(((i % SPEED_TICK_SPACING) == 0) ? b : b2);
                }
                if (b >= hud_settings.ledbrite_tick)
                {
                    dbg_ser.printf("[%u]: ANI fade-in %d end -> speedometer\r\n", millis(), hud_animation);
                    hud_animation = HUDANI_SPEEDOMETER;
                }
                else
                {
                    hud_aniStep += 1;
                }
            }
            break;
        case HUDANI_FADEOUT_ALL:
            {
                strip_aniFadeOutReport();
                hud_aniDelay = 20;
                need_show = true;
                int b = hud_settings.ledbrite_tick - (hud_aniStep * 4);
                b = b < 0 ? 0 : b;
                int i;
                for (i = 0; i < LED_STRIP_SIZE; i++)
                {
                    leds[i] = CRGB_BLUE(((i % SPEED_TICK_SPACING) == 0) ? b : 0);
                }
                if (b <= 0)
                {
                    dbg_ser.printf("[%u]: ANI fade-out %d end\r\n", millis(), hud_animation);
                    hud_animation = HUDANI_FADEOUT_END;
                }
                else
                {
                    hud_aniStep += 1;
                }
            }
            break;
        case HUDANI_FADEIN_FADESCROLL:
            {
                strip_aniFadeInReport();
                hud_aniDelay = 50;
                need_show = true;
                int fully_lit = hud_aniStep / 256;
                int last_brightness = hud_aniStep % 256;
                int i, j;
                for (i = 0, j = 0; i < LED_STRIP_SIZE && j < fully_lit; i += SPEED_TICK_SPACING, j++)
                {
                    leds[i] = CRGB_BLUE(hud_settings.ledbrite_tick);
                }
                if (i < LED_STRIP_SIZE)
                {
                    leds[i] = CRGB_BLUE(last_brightness);
                    hud_aniStep += 8;
                }
                else
                {
                    dbg_ser.printf("[%u]: ANI fade-in %d done -> speedometer\r\n", millis(), hud_animation);
                    hud_animation = HUDANI_SPEEDOMETER;
                }
            }
            break;
        case HUDANI_FADEOUT_SEQ:
            {
                strip_aniFadeOutReport();
                hud_aniDelay = 50;
                need_show = true;
                int fully_dim = hud_aniStep / 256;
                int last_brightness = 255 - (hud_aniStep % 256);
                int i, j;
                if (hud_aniStep == 0) {
                    strip_speedometer(0);
                }
                for (i = LED_STRIP_SIZE - 1, j = 0; i >= 0 && j < fully_dim; i -= SPEED_TICK_SPACING, j++)
                {
                    leds[i] = CRGB_BLUE(0);
                }
                if (i >= 0)
                {
                    leds[i] = CRGB_BLUE(last_brightness);
                    hud_aniStep += 8;
                }
                else
                {
                    dbg_ser.printf("[%u]: ANI fade-out %d done\r\n", millis(), hud_animation);
                    hud_animation = HUDANI_FADEOUT_END;
                }
            }
            break;
        case HUDANI_FADEIN_SCROLL:
        case HUDANI_FADEIN_SCROLLFADE:
            {
                strip_aniFadeInReport();
                hud_aniDelay = 2000 / LED_STRIP_SIZE;
                need_show = true;
                int i;
                volatile int brite = hud_settings.ledbrite_tick;
                if (hud_animation == HUDANI_FADEIN_SCROLLFADE) {
                    brite *= hud_aniStep;
                    brite /= LED_STRIP_SIZE;
                    brite = brite > hud_settings.ledbrite_tick ? hud_settings.ledbrite_tick : brite;
                }
                strip_blank();
                for (i = hud_aniStep; i >= 0; i -= SPEED_TICK_SPACING)
                {
                    leds[i] = CRGB_BLUE(brite);
                }
                hud_aniStep += 1;
                if (hud_aniStep >= LED_STRIP_SIZE) {
                    dbg_ser.printf("[%u]: ANI fade-in %d done -> speedometer\r\n", millis(), hud_animation);
                    hud_animation = HUDANI_SPEEDOMETER;
                }
            }
            break;
        case HUDANI_FADEIN_RANDOM:
        case HUDANI_FADEOUT_RANDOM:
            {
                static int16_t rnd_pick;
                static int16_t rnd_cnt;
                bool did = false;
                if (hud_aniStep == 0)
                {
                    dbg_ser.printf("[%u]: ANI fade %d start\r\n", millis(), hud_animation);

                    if (hud_animation == HUDANI_FADEIN_RANDOM) {
                        strip_blank();
                    }
                    else {
                        strip_speedometer(0); // forces all ticks to be displayed immediately
                    }
                    rnd_pick = rand() % 9;
                    rnd_cnt = 1;
                }
                int i = rnd_pick * SPEED_TICK_SPACING;
                int16_t old_b = leds[i].b;
                if ((hud_animation == HUDANI_FADEIN_RANDOM && old_b < 255) || (hud_animation == HUDANI_FADEOUT_RANDOM && old_b > 0))
                {
                    int16_t new_b = old_b + (8 * (hud_animation == HUDANI_FADEIN_RANDOM ? 1 : -1));
                    new_b = new_b > 255 ? 255 : new_b;
                    new_b = new_b < 0 ? 0 : new_b;
                    leds[i] = CRGB_BLUE(new_b);
                    did = true;
                    if ((hud_animation == HUDANI_FADEIN_RANDOM && new_b >= 255) || (hud_animation == HUDANI_FADEOUT_RANDOM && new_b <= 0))
                    {
                        if (rnd_cnt < 9)
                        {
                            while (true)
                            {
                                rnd_pick = rand() % 9;
                                i = rnd_pick * SPEED_TICK_SPACING;
                                if ((hud_animation == HUDANI_FADEIN_RANDOM && leds[i].b <= 0) || (hud_animation == HUDANI_FADEOUT_RANDOM && leds[i].b > 0))
                                {
                                    rnd_cnt++;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            hud_animation = (hud_animation == HUDANI_FADEIN_RANDOM) ? HUDANI_SPEEDOMETER : HUDANI_FADEOUT_END;
                            dbg_ser.printf("[%u]: ANI fade random end -> %d\r\n", millis(), hud_animation);
                        }
                    }
                }
                hud_aniStep += 1;
                if (did)
                {
                    hud_aniDelay = 50;
                    need_show = true;
                }
            }
            break;
        case HUDANI_FADEIN_FUZZ:
            {
                strip_aniFadeInReport();
                if (strip_hasAllTicks()) {
                    dbg_ser.printf("[%u]: ANI fade-in %d done -> speedometer\r\n", millis(), hud_animation);
                    hud_animation = HUDANI_SPEEDOMETER;
                    hud_aniStep = 0;
                    break;
                }
                bool did = false;
                int i;
                while (did == false)
                {
                    i = (rand() % 9) * SPEED_TICK_SPACING;
                    int bi = (rand() % 32) - 8;
                    if (leds[i].b < 255)
                    {
                        int16_t old_b = leds[i].b;
                        int16_t new_b = old_b + bi;
                        new_b = new_b > hud_settings.ledbrite_tick ? hud_settings.ledbrite_tick : new_b;
                        new_b = new_b < (hud_settings.ledbrite_tick / 8) ? (hud_settings.ledbrite_tick / 8) : new_b;
                        leds[i] = CRGB_BLUE(new_b);
                        did = true;
                        break;
                    }
                }
                hud_aniStep += 1;
                if (did)
                {
                    hud_aniDelay = 50;
                    need_show = true;
                }
            }
            break;
        case HUDANI_FADEIN_STATIC:
            {
                strip_aniFadeInReport();
                if (strip_isAllTicks()) {
                    dbg_ser.printf("[%u]: ANI fade-in %d done -> speedometer\r\n", millis(), hud_animation);
                    hud_animation = HUDANI_SPEEDOMETER;
                    hud_aniStep = 0;
                    break;
                }
                static uint32_t start_time;
                static int16_t rnd_pick;
                static bool need_fast;
                bool did = false;
                if (hud_aniStep == 0) {
                    need_fast = false;
                    rnd_pick = -1;
                    start_time = millis();
                    hud_aniStep = 1;
                }

                need_fast |= ((start_time - millis()) > 1500) || car_data.rpm > 0;

                if (rnd_pick < 0)
                {
                    int r;
                    while (did == false)
                    {
                        r = rand() % LED_STRIP_SIZE;
                        if (leds[r].b <= 0)
                        {
                            did = true;
                            if ((r % SPEED_TICK_SPACING) == 0)
                            {
                                leds[r] = CRGB_BLUE(hud_settings.ledbrite_tick);
                                hud_aniDelay = 150 + (rand() % 100);
                            }
                            else
                            {
                                rnd_pick = r;
                                int b = hud_settings.ledbrite_tick * 3 / 4;
                                leds[r] = CRGB_BLUE(b);
                                hud_aniDelay = need_fast ? 25 : 50;
                            }
                            break;
                        }
                    }
                }
                else
                {
                    int16_t old_b = leds[rnd_pick].b;
                    int16_t new_b = old_b - 32;
                    new_b = new_b < 0 ? 0 : new_b;
                    leds[rnd_pick] = CRGB_BLUE(new_b);
                    if (new_b <= 0)
                    {
                        rnd_pick = -1;
                    }
                    did = true;
                    hud_aniDelay = need_fast ? 25 : 50;
                }

                if (need_fast != false)
                {
                    did = true;
                    int i;
                    for (i = 0; i < LED_STRIP_SIZE; i += SPEED_TICK_SPACING)
                    {
                        int16_t old_b = leds[i].b;
                        int16_t new_b = old_b + 4;
                        new_b = new_b > 255 ? 255 : new_b;
                        leds[i] = CRGB_BLUE(new_b);
                    }
                    hud_aniDelay = 25;
                }
                if (did)
                {
                    need_show = true;
                }
            }
            break;
        case HUDANI_VOLTMETER_FADEIN:
            {
                strip_aniFadeInReport();
                int b = hud_aniStep * 2;
                b = b > hud_settings.ledbrite_volt ? hud_settings.ledbrite_volt : b;
                float v = car_data.aux_batt_volt_x10;
                v = mapf(b, 0, hud_settings.ledbrite_volt, 11.5, v / 10.0);
                strip_voltmeter(v, b);
                need_show = true;
                hud_aniStep++;
                hud_aniDelay = 25;
                if (b >= hud_settings.ledbrite_volt) {
                    hud_animation = HUDANI_VOLTMETER;
                    hud_aniStep = 0;
                    dbg_ser.printf("[%u]: ANI voltmeter fade-in done\r\n", millis());
                }
            }
            break;
        case HUDANI_VOLTMETER:
            {
                float v = car_data.aux_batt_volt_x10;
                strip_voltmeter(v / 10, hud_settings.ledbrite_volt);
                need_show = true;
                hud_aniDelay = 200;
                hud_aniStep++;
                if (hud_aniStep >= 5 * 5) {
                    hud_animation = HUDANI_VOLTMETER_FADEOUT;
                    hud_aniStep = 0;
                    dbg_ser.printf("[%u]: ANI voltmeter done\r\n", millis());
                }
            }
            break;
        case HUDANI_VOLTMETER_FADEOUT:
            {
                strip_aniFadeOutReport();
                float v = car_data.aux_batt_volt_x10;
                int b = hud_settings.ledbrite_volt - (hud_aniStep * 4);
                b = b < 0 ? 0 : b;
                strip_voltmeter(v / 10, b);
                need_show = true;
                hud_aniDelay = 25;
                if (b <= 0)
                {
                    hud_animation = hud_animation_queue;
                    dbg_ser.printf("[%u]: ANI voltmeter fade-out -> %u\r\n", millis(), hud_animation);
                    hud_animation_queue = HUDANI_OFF;
                    hud_aniStep = 0;
                }
            }
            break;
    }

    if (need_show)
    {
        int dim_briteness = amblight_get();
        strip_ctrler->showLeds((uint8_t)dim_briteness);
        #ifdef DEBUG_STRIP
        strip_debug();
        #endif
    }
}

void strip_debug()
{
    int i;
    Serial.println();
    Serial.print("LEDS: ");
    for (i = 0; i < LED_STRIP_SIZE; i++)
    {
        uint8_t maxb = 0;
        maxb = leds[i].r > maxb ? leds[i].r : maxb;
        maxb = leds[i].g > maxb ? leds[i].g : maxb;
        maxb = leds[i].b > maxb ? leds[i].b : maxb;
        Serial.printf("%u", map(maxb, 0, 255, 0, 9));
    }
    Serial.println();
}
