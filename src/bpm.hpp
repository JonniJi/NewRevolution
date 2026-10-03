#ifndef BPM_HPP
#define BPM_HPP

#include <Arduino.h>
#include <FastLED.h>
#include "defines.h"

class BpmAnimation : public IAnimation
{
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    uint8_t gHue = 0;

public:
    BpmAnimation() {}

    void OnHallEvent(struct ledData data)  {
        // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
        data.leds[data.pos] = ColorFromPalette(palette, gHue + (data.pos * 1), beat - gHue + (data.pos * 30));
    };
    void OnSetup() {}
    void OnFastLoop() {}
    String Name() { return "BPM"; }
};

#endif
