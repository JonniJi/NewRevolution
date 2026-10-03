#ifndef HEARTBEAT_HPP
#define HEARTBEAT_HPP

#include <Arduino.h>
#include <FastLED.h>
#include "defines.h"

class HeartBeatAnimation : public IAnimation
{
    uint8_t gHue = 0;

public:
    HeartBeatAnimation() {}

    void OnHallEvent(struct ledData data)  {
        // a colored dot sweeping back and forth, with fading trails
        //fadeToBlackBy(data.leds, data.noOfLeds, 20);
        // int pos = beatsin16(13, 0, data.noOfLeds - 1);

        //alternative comment out to be red
        gHue = beatsin8(0, 8, data.noOfLeds - 1);

        for (uint16_t i = 0; i < data.noOfLeds; ++i)
        {
            data.leds[i] = CHSV(gHue + (i * 10), 255, 255);
        }

        gHue += 20;
        //rainbowHue += 20;

        //data.leds[data.pos] += CHSV(gHue, 255, 192);
        //alternating
        //data.leds[pos] += CHSV(gHue, 255, 192);
    };
    void OnSetup() {}
    void OnFastLoop() {}
    String Name() { return "HeartBeat"; }
};

#endif
