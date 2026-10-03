#ifndef RAINBOW2ANIMATION_HPP
#define RAINBOW2ANIMATION_HPP

#include <Arduino.h>
#include <FastLED.h>
#include "defines.h"

class Rainbow2LedAnimation : public IAnimation
{
    
    uint8_t rainbowHue = 0;

public:
    Rainbow2LedAnimation() {}
    void OnHallEvent(struct ledData data)
    {
        fill_rainbow(data.leds, data.noOfLeds, 7);
        rainbowHue += 10;
    };
    void OnSetup(){}
    void OnFastLoop() {}
    String Name() { return "Rainbow2"; }
};
#endif