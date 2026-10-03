#ifndef CANDYCANE_HPP
#define CANDYCANE_HPP

#include <Arduino.h>
#include <FastLED.h>
#include "defines.h"

class CandyCaneAnimation : public IAnimation
{
    uint8_t numColors = 2;    // Can be either 2 or 3
    uint16_t stripeLength = 4; // number of pixels per color
    CRGB color1 = CRGB::Blue;         // color used between color 2 (and 3 if used)
    CRGB color2 = CRGB::Black;

public:
    CandyCaneAnimation() {}

    void OnHallEvent(struct ledData data)
    {
        for (uint16_t i = 0; i < data.noOfLeds; i++)
        {
            if ((i + data.pos) % ((numColors)*stripeLength) < stripeLength)
            {
                data.leds[i] = color2;
            }
            else
            {
                data.leds[i] = color1;
            }
        }
    };
    void OnSetup() {}
    void OnFastLoop() {}
    String Name() { return "CandyCane"; }
};


#endif