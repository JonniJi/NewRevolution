#ifndef BLUEANDWHITE_HPP
#define BLUEANDWHITE_HPP

#include <Arduino.h>
#include <FastLED.h>
#include "defines.h"

class BlueAndWhiteAnimation : public IAnimation
{
    uint16_t spacing;
    uint16_t width;
    uint8_t colorOne;
    uint8_t colorTwo;

public:
    BlueAndWhiteAnimation(CRGB colorOne = CRGB::White, CRGB colorTwo = CRGB::Blue, uint8_t spacing = 3, uint8_t width = 1)
    {
        this->spacing = spacing;
        this->width = width;
        this->colorOne = colorOne;
        this->colorTwo = colorTwo;
    }

    void OnHallEvent(struct ledData data){


        for (uint16_t i = 0; i < data.noOfLeds; i = (i + spacing))
        {
            if ((i + data.pos) % 2)
            {
                data.leds[i] = colorOne;
            }
            else
            {
                data.leds[i] = colorTwo;
            }
        }
    };
    void OnSetup() {}
    void OnFastLoop() {}
    String Name() { return "BlueAndWhite"; }
};

#endif


