#include <Arduino.h>
#include <rotate.hpp>
#include <heartBeat.hpp>
#include <sinelon.hpp>
#include <bpm.hpp>
#include <candyCane.hpp>
#include <rainbow2.hpp>
#include <rainbow.hpp>
#include <snake.hpp>
#include <whiteDotRunning.hpp>
#include <blueAndWhite.hpp>
#include <juggle.hpp>

#include "QList.h"
#include "defines.h"
#define FASTLED_INTERRUPT_RETRY_COUNT 2
// #define FASTLED_ALLOW_INTERRUPTS 0 // Needed to fix jitter/fluttering!
#include "FastLED.h"

CRGB ledsOne[NUM_LEDS_ONE]; // the led array representing the leds addressable by FASTLED
#ifdef LED_STRING_TWO_PRESENT
CRGB ledsTwo[NUM_LEDS_TWO]; // the led array representing the leds addressable by FASTLED
#endif

uint8_t numberOfAnimations = 0;
uint8_t numberOfPotPositions = 0;

IAnimation *activeAnimationOne;
IAnimation *activeAnimationTwo;
IAnimation *activeAnimationStationaryOne;
IAnimation *activeAnimationStationaryTwo;

class PotPosLimits
{
public:
  PotPosLimits() {}

  PotPosLimits(uint16_t st, uint16_t sp)
  {
    start = st;
    stop = sp;
  }
  uint16_t start = 0;
  uint16_t stop = 0;
};

PotPosLimits potPositions[12];

IAnimation *ledFunctionsOne[12];
IAnimation *ledFunctionsWhenStationaryOne[12];
#ifdef LED_STRING_TWO_PRESENT
IAnimation *ledFunctionsWhenStationaryTwo[12];
IAnimation *ledFunctionsTwo[12];
#endif

uint16_t deltaOne = 1; // Sets forward or backwards direction amount. (Can be negative.) (delta = 1 WORKS)
#ifdef LED_STRING_TWO_PRESENT
uint16_t deltaTwo = 1; // Sets forward or backwards direction amount. (Can be negative.) (delta = 1 WORKS)
#endif

bool startUpDone = false;

struct ledData ledDataOne;
#ifdef LED_STRING_TWO_PRESENT
struct ledData ledDataTwo;
#endif

bool toggle = false;  // used to limit the number of hall events per magnet to one
bool reverse = false; // the direction of the hall event

QList<bool> eventList; // has all interrupt events
bool lock = false;
byte potPin = A0;
uint16_t potVal = analogRead(A0);
bool hallSensorActive = true;
bool simulateHallSensorEvents = true;

void interruptHall()
{
  if (startUpDone == false)
  {
    return;
  }

  hallSensorActive = true;
  // make sure there is only one event per magnet
  if (toggle == false)
  {
    toggle = true;
    eventList.push_back(true);
  }
  else
  {
    toggle = false;
  }
#ifdef SHOW_LOCK_AND_QUEUE_INFO
  Serial.println("------------------------------>");
  Serial.println(eventList.length());
  Serial.print("lock:");
  Serial.println(lock ? "true" : "false");
#endif
}
void interruptHallSimulated()
{
  eventList.push_back(true);
}

uint8_t potPosition = 0;

void ledCodeOnHalEvent()
{

  activeAnimationOne->OnHallEvent(ledDataOne);

#ifdef LED_STRING_TWO_PRESENT
  activeAnimationTwo->OnHallEvent(ledDataTwo);
#endif
}

void ledCodeOnLoop()
{
  activeAnimationOne->OnFastLoop();

#ifdef LED_STRING_TWO_PRESENT
  activeAnimationTwo->OnFastLoop();
#endif
}

void ledCodeOnSetup()
{
  for (int i; i < numberOfAnimations; i++)
  {
    ledFunctionsOne[i]->OnSetup();
    ledFunctionsWhenStationaryOne[i]->OnSetup();
  }

#ifdef LED_STRING_TWO_PRESENT
  for (int i; i < numberOfAnimations; i++)
  {
    ledFunctionsTwo[i]->OnSetup();
    ledFunctionsWhenStationaryTwo[i]->OnSetup();
  }
#endif
}

void showStartupAnimation()
{
  ledsOne[0] = CRGB::Red;
#ifdef LED_STRING_TWO_PRESENT
  ledsTwo[0] = CRGB::Red;
#endif
  FastLED.show();
  Serial.println("red");

  delay(2500);
  ledsOne[1] = CRGB::Orange;
#ifdef LED_STRING_TWO_PRESENT
  ledsTwo[1] = CRGB::Orange;
#endif
  FastLED.show();
  Serial.println("orange");

  delay(2500);
  ledsOne[2] = CRGB::Yellow;
#ifdef LED_STRING_TWO_PRESENT
  ledsTwo[2] = CRGB::Yellow;
#endif
  FastLED.show();
  Serial.println("yellow");

  delay(2500);
  ledsOne[3] = CRGB::Green;
#ifdef LED_STRING_TWO_PRESENT
  ledsTwo[3] = CRGB::Green;
#endif
  Serial.println("green");
  FastLED.show();
  delay(2500);
  FastLED.clear();
}

void initPot()
{
  potPositions[0] = PotPosLimits(0, 50);
  potPositions[1] = PotPosLimits(51, 149);
  potPositions[2] = PotPosLimits(150, 200);
  potPositions[3] = PotPosLimits(201, 300);
  potPositions[4] = PotPosLimits(301, 400);
  potPositions[5] = PotPosLimits(401, 500);
  potPositions[6] = PotPosLimits(501, 600);
  potPositions[7] = PotPosLimits(601, 700);
  potPositions[8] = PotPosLimits(701, 800);
  potPositions[9] = PotPosLimits(801, 900);
  potPositions[11] = PotPosLimits(901, 1200);
}

void UpdatePotPosition()
{
  PotPosLimits limits;
  potVal = analogRead(potPin);
#ifdef SHOW_POTENTIOMETER_INFO
  Serial.print("potVal - read:");
  Serial.println(potVal);
#endif

  for (uint8_t i = 0; i < numberOfPotPositions; i++)
  {
    limits = potPositions[i];
    if (potVal >= limits.start && potVal < limits.stop)
    {
      potPosition = i;
      break;
    }
  }

  activeAnimationOne = ledFunctionsOne[potPosition];
  activeAnimationStationaryOne = ledFunctionsWhenStationaryOne[potPosition];

#ifdef LED_STRING_TWO_PRESENT
  activeAnimationTwo = ledFunctionsTwo[potPosition];
  activeAnimationStationaryTwo = ledFunctionsWhenStationaryTwo[potPosition];
#endif

#ifdef SHOW_POTENTIOMETER_INFO

  Serial.print("potVal:");
  Serial.println(potVal);
  Serial.print("pot Position:");
  Serial.println(potPosition);
  Serial.print("Limit start: ");
  Serial.println(limits.start);
  Serial.print("Limit stop: ");
  Serial.println(limits.stop);
  Serial.print("Active Animation One: ");
  Serial.println(activeAnimationOne->Name());

#endif
}
void UpdatePosition(struct ledData &data, uint16_t delta)
{
#ifdef SHOW_POSITION_PRINT_INFO
  Serial.print("pos one before:");
  Serial.println(data.pos);
#endif

  data.lastPos = data.pos;

  if (reverse)
  {
    // backwards
    if (delta > data.pos)
    {
      data.pos = data.noOfLeds - (delta - data.pos);
    }
    else
    {
      data.pos = data.pos - delta;
    }
  }
  else
  {
    // forwards
    if ((data.pos + delta) < data.noOfLeds)
    {
      data.pos = data.pos + delta;
      // Serial.print("pos + delta:");
      // Serial.println(pos);
    }
    else
    {
      data.pos = data.pos + delta - data.noOfLeds;
      // Serial.print("pos + delta - NUM_LEDS_ONE:");
      // Serial.println(pos);
    }
  }
#ifdef SHOW_POSITION_PRINT_INFO
  Serial.print("pos one after:");
  Serial.println(data.pos);
#endif
}

void loop()
{
  // slow updates here ...
  EVERY_N_MILLISECONDS(30000) // Advance pixels to next position.
  {

#ifdef AUTO_SELECT_ANIMATION
    if (potPosition < (numberOfAnimations - 1))
      potPosition++;
    else
      potPosition = 0;
#endif
  }

  // no need to read the Potentiometer often
  EVERY_N_MILLISECONDS(1000)
  {
#ifndef AUTO_SELECT_ANIMATION
    UpdatePotPosition();
#endif
  }

  EVERY_N_MILLISECONDS(5000)
  {
    if (hallSensorActive == true)
    {
      hallSensorActive = false;
      simulateHallSensorEvents = false;
    }
    else
    {
      simulateHallSensorEvents = true;
    }
  }

  EVERY_N_MILLISECONDS(SIMULATED_INTERRUPT_TIME) // Advance pixels to next position.
  {
    if (simulateHallSensorEvents)
      interruptHallSimulated();
  }

  if (lock == false)
  {

    // lock is making sure that the led routine gets not entered when it
    // is already running (avoid crash and reentrance)
    lock = true;

    ledCodeOnLoop();

    if (eventList.length() != 0)
    {
      UpdatePosition(ledDataOne, deltaOne);
#ifdef LED_STRING_TWO_PRESENT
      UpdatePosition(ledDataTwo, deltaTwo);
#endif
      ledCodeOnHalEvent();
      FastLED.show();

      // Serial.println("led done");

      // que makes sure that we dont miss interrupt events
      eventList.pop_back();
      // digitalToggleFast(LED_BUILTIN);
    }

    lock = false;
  }
}

void initAnimations()
{
  ledFunctionsOne[0] = new RotateAnimation(ColorMode::TwoColorFade, CRGB::Blue, CRGB::Red, 16, 3, 100, NUM_LEDS_ONE);
  ledFunctionsOne[1] = new RotateAnimation(ColorMode::OneColorFade, CRGB::Blue, CRGB::Red, 16, 3, 100, NUM_LEDS_ONE);
  ledFunctionsOne[2] = new RotateAnimation(ColorMode::TwoColorFade, CRGB::Blue, CRGB::Red, 16, 3, 150, NUM_LEDS_ONE);
  ledFunctionsOne[3] = new RotateAnimation(ColorMode::OneColorFade, CRGB::Blue, CRGB::Red, 16, 3, 150, NUM_LEDS_ONE);
  ledFunctionsOne[4] = new RotateAnimation(ColorMode::TwoColor, CRGB::Blue, CRGB::Red, 8, 3, 150, NUM_LEDS_ONE);
  ledFunctionsOne[5] = new RotateAnimation(ColorMode::TwoColor, CRGB::Blue, CRGB::Red, 8, 3, 150, NUM_LEDS_ONE);
  ledFunctionsOne[6] = new RotateAnimation(ColorMode::RainBow, CRGB::Blue, CRGB::Red, 8, 3, 50, NUM_LEDS_ONE);
   ledFunctionsOne[7] = new JuggleAnimation();
  ledFunctionsOne[8] = new CandyCaneAnimation();
  ledFunctionsOne[9] = new BlueAndWhiteAnimation(HUE_RED, HUE_RED, 3, 3);
  //ledFunctionsOne[4] = new HeartBeatAnimation();
  ledFunctionsOne[10] = new SinelonAnimation();
  //ledFunctionsOne[6] = new BpmAnimation();
  // ledFunctionsOne[9] = new WhiteDotRunningAnimation();
  //ledFunctionsOne[8] = new Rainbow2LedAnimation();
  // ledFunctionsOne[10] = new RainbowLedAnimation();

  ledFunctionsWhenStationaryOne[0] = new RotateAnimation(ColorMode::OneColor, CRGB::Blue, CRGB::Red, 20, 6, 2, NUM_LEDS_ONE);
  ledFunctionsWhenStationaryOne[1] = new JuggleAnimation();
  ledFunctionsWhenStationaryOne[2] = new CandyCaneAnimation();
  ledFunctionsWhenStationaryOne[3] = new BlueAndWhiteAnimation(HUE_RED, HUE_RED, 3, 3);
  ledFunctionsWhenStationaryOne[4] = new HeartBeatAnimation();
  ledFunctionsWhenStationaryOne[5] = new SinelonAnimation();
  ledFunctionsWhenStationaryOne[6] = new BpmAnimation();
  ledFunctionsWhenStationaryOne[7] = new WhiteDotRunningAnimation();
  ledFunctionsWhenStationaryOne[8] = new Rainbow2LedAnimation();
  ledFunctionsWhenStationaryOne[9] = new RainbowLedAnimation();
  ledFunctionsWhenStationaryOne[10] = new RainbowLedAnimation();

  ;
#ifdef LED_STRING_TWO_PRESENT
  ledFunctionsTwo[0] = new RotateAnimation(ColorMode::OneColorFade, CRGB::Blue, CRGB::Red, 3, 1, 2, NUM_LEDS_TWO);
  ledFunctionsTwo[1] = new JuggleAnimation();
  ledFunctionsTwo[2] = new CandyCaneAnimation();
  ledFunctionsTwo[3] = new BlueAndWhiteAnimation(CRGB::Blue, CRGB::Red, 3, 3);
  ledFunctionsTwo[4] = new HeartBeatAnimation();
  ledFunctionsTwo[5] = new SinelonAnimation();
  ledFunctionsTwo[6] = new BpmAnimation();
  ledFunctionsTwo[7] = new WhiteDotRunningAnimation();
  ledFunctionsTwo[8] = new Rainbow2LedAnimation();
  ledFunctionsTwo[9] = new RainbowLedAnimation();
  ledFunctionsTwo[10] = new RainbowLedAnimation();

  ledFunctionsWhenStationaryTwo[0] = new RotateAnimation(ColorMode::OneColor, CRGB::Blue, CRGB::Red, 4, 2, 2, NUM_LEDS_TWO);
  ledFunctionsWhenStationaryTwo[1] = new JuggleAnimation();
  ledFunctionsWhenStationaryTwo[2] = new CandyCaneAnimation();
  ledFunctionsWhenStationaryTwo[3] = new BlueAndWhiteAnimation(CRGB::Black, CRGB::Blue, 3, 3);
  ledFunctionsWhenStationaryTwo[4] = new HeartBeatAnimation();
  ledFunctionsWhenStationaryTwo[5] = new SinelonAnimation();
  ledFunctionsWhenStationaryTwo[6] = new BpmAnimation();
  ledFunctionsWhenStationaryTwo[7] = new WhiteDotRunningAnimation();
  ledFunctionsWhenStationaryTwo[8] = new Rainbow2LedAnimation();
  ledFunctionsWhenStationaryTwo[9] = new RainbowLedAnimation();
  ledFunctionsWhenStationaryTwo[10] = new RainbowLedAnimation();

#endif
}

void setup()
{

  initAnimations();

  initPot();

  // numberOfAnimations = sizeof(ledFunctionsOne) / sizeof(ledFunctionsOne[0]);
  numberOfAnimations = 10;
  numberOfPotPositions = 12;

#ifndef AUTO_SELECT_ANIMATION
  UpdatePotPosition();
#endif

  // sizeof(potPosition) / sizeof(potPositions[0]); //results in 0

  ledDataOne = {ledsOne, 0, 0, NUM_LEDS_ONE};
#ifdef LED_STRING_TWO_PRESENT
  ledDataTwo = {ledsTwo, 0, 0, NUM_LEDS_TWO};
#endif

  Serial.begin(115200);
  Serial.println("Setup start --------------------------------------");
  Serial.print("Number of Pot Positions:");
  Serial.println(numberOfPotPositions);
  Serial.print("Number of Animations:");
  Serial.println(numberOfAnimations);

  FastLED.clear();
  delay(1600); // Startup delay

  pinMode(HALL_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  // setup Interrupt
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), interruptHall, CHANGE);

  Serial.println("1 -------------");

  FastLED.addLeds<LED_TYPE, DATA_PIN_ONE, GRB>(ledsOne, NUM_LEDS_ONE);
#ifdef LED_STRING_TWO_PRESENT
  FastLED.addLeds<LED_TYPE, DATA_PIN_TWO, GRB>(ledsTwo, NUM_LEDS_TWO);
#endif

  // 4 Wire LED setup
  // FastLED.addLeds<LED_TYPE, DATA_PIN_ONE, CLK_PIN, COLOR_ORDER>(ledsOne, NUM_LEDS_ONE).setCorrection(TypicalLEDStrip); // with clock_pin

  Serial.println("2 -------------");

  FastLED.setBrightness(BRIGHTNESS);

  Serial.println("3 -------------");

  showStartupAnimation();

  ledCodeOnSetup();

  Serial.println("6 -------------");

  Serial.print("NUM_LEDS_ONE: ");
  Serial.println(NUM_LEDS_ONE);
  Serial.print("no Of Leds: ");
  Serial.println(ledDataOne.noOfLeds);

  Serial.println("Setup done --------------------------------------");

  startUpDone = true;
}
