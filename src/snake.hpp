#ifndef SNAKE_HPP
#define SNAKE_HPP

#include <Arduino.h>
#include <FastLED.h>
#include "defines.h"

class SnakeAnimation : public IAnimation
{
    uint8_t snakeLength = 70;
    CRGB background = CRGB::Blue; // color used between color 2 (and 3 if used)
    CRGB foreground = CRGB::Black;
    uint8_t snakeHue = 0;

public:
    SnakeAnimation() {}

    void OnHallEvent(struct ledData data) 
    {
        //     Serial.println("snakeHallEvent---------------------------------------");
        //     Serial.println(data.pos);
        //     Serial.println(data.lastPos);
        //     Serial.println(data.noOfLeds);
        //     Serial.println("end snakeHallEvent---------------------------------------");

        uint8_t oldHue = snakeHue;

        // paint background
        for (uint16_t i = 0; i < data.noOfLeds; i++)
        {

            data.leds[i] = background;
        }
        Serial.println("snake 1---------------------------------");

        if (data.pos > snakeLength)
        {
            Serial.println("snake 2---------------------------------");
            // snake until the end
            for (uint16_t i = 0; i < snakeLength; i++)
            {
                data.leds[data.pos - i] = CHSV(snakeHue++, 255, 255);
            }
        }
        else
        {
            Serial.println("snake 3---------------------------------");
            // loop for the few LEds on the beginning
            for (uint16_t i = data.pos; i >= 0; i--)
            {
                data.leds[i] = CHSV(snakeHue++, 255, 255);
            }
            Serial.println("snake 4---------------------------------");
            // loop for the few LEds on the beginning
            for (uint16_t i = data.pos + data.noOfLeds - snakeLength; i < data.noOfLeds; i++)
            {
                data.leds[i] = CHSV(snakeHue++, 255, 255);
            }
        }
        Serial.println("snake 5---------------------------------");
        snakeHue = oldHue + 10;
    };
    void OnSetup() {}
    void OnFastLoop() {}
    String Name() { return "Snake"; }
};
#endif
