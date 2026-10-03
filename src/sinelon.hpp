#ifndef SINELON_HPP
#define SINELON_HPP

#include <Arduino.h>
#include <FastLED.h>
#include "defines.h"

class SinelonAnimation : public IAnimation
{
    uint8_t gHue = 0;

public:
    SinelonAnimation() {}

    void OnHallEvent(struct ledData data)  {
        // a colored dot sweeping back and forth, with fading trails
        fadeToBlackBy(data.leds, data.noOfLeds, 20);
        int pos = beatsin16(13, 0, data.noOfLeds - 1);

        //alternative comment out to be red
        gHue = beatsin8(13, 0, data.noOfLeds - 1);
        
        data.leds[data.pos] += CHSV(gHue, 255, 192);
        //alternating
        //data.leds[pos] += CHSV(gHue, 255, 192);
    };
    void OnSetup() {}
    void OnFastLoop() {}
    String Name() { return "Sinelon"; }
};

#endif
