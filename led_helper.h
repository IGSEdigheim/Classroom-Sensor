// JLed multi LED demo. control multiple LEDs in-sync.
// Copyright (c) 2017-2021 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

#define LED1B 12
#define LED1G 14
#define LED1R 27
#define LED2B 25
#define LED2G 33
#define LED2R 32

#define FADE_TIME 1000

#define LEDS_OFF 0
#define RED_ON 1
#define YELLOW_ON 2
#define GREEN_ON 3
#define BLUE_ON 4

JLed led1b = JLed(LED1B);
JLed led1g = JLed(LED1G);
JLed led1r = JLed(LED1R);
JLed led2b = JLed(LED2B);
JLed led2g = JLed(LED2G);
JLed led2r = JLed(LED2R);

JLed leds1_off[] = {
    led1b.Off(),
    led1g.Off(),
    led1r.Off()
};

JLed leds1_red_on[] = {
    led1b.Off(),
    led1g.Off(),
    led1r.FadeOn(FADE_TIME)
};

JLed leds1_yellow_on[] = {
    led1b.Off(),
    led1g.FadeOn(FADE_TIME),
    led1r.FadeOn(FADE_TIME)
};

JLed leds1_green_on[] = {
    led1b.Off(),
    led1g.FadeOn(FADE_TIME),
    led1r.Off()
};

JLed leds1_blue_on[] = {
    led1b.FadeOn(FADE_TIME),
    led1g.Off(),
    led1r.Off()
};

JLed leds2_off[] = {
    led2b.Off(),
    led2g.Off(),
    led2r.Off()
};

JLed leds2_red_on[] = {
    led2b.Off(),
    led2g.Off(),
    led2r.FadeOn(FADE_TIME)
};

JLed leds2_yellow_on[] = {
    led2b.Off(),
    led2g.FadeOn(FADE_TIME),
    led2r.FadeOn(FADE_TIME)
};

JLed leds2_green_on[] = {
    led2b.Off(),
    led2g.FadeOn(FADE_TIME),
    led2r.Off()
};

JLed leds2_blue_on[] = {
    led2b.FadeOn(FADE_TIME),
    led2g.Off(),
    led2r.Off()
};

JLedSequence leds1[] = {
  JLedSequence(JLedSequence::eMode::PARALLEL, leds1_off),
  JLedSequence(JLedSequence::eMode::PARALLEL, leds1_red_on),
  JLedSequence(JLedSequence::eMode::PARALLEL, leds1_yellow_on),
  JLedSequence(JLedSequence::eMode::PARALLEL, leds1_green_on),
  JLedSequence(JLedSequence::eMode::PARALLEL, leds1_blue_on)
};

JLedSequence leds2[] = {
  JLedSequence(JLedSequence::eMode::PARALLEL, leds2_off),
  JLedSequence(JLedSequence::eMode::PARALLEL, leds2_red_on),
  JLedSequence(JLedSequence::eMode::PARALLEL, leds2_yellow_on),
  JLedSequence(JLedSequence::eMode::PARALLEL, leds2_green_on),
  JLedSequence(JLedSequence::eMode::PARALLEL, leds2_blue_on)
};

int iLeds1 = LEDS_OFF;
int iLeds2 = LEDS_OFF;
