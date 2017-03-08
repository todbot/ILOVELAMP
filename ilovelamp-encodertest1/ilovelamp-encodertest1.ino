/* Encoder Library - TwoKnobs Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 */

#include <Encoder.h>
#include <TaskScheduler.h>
#include "FastLED.h"

const int led1Pin = 2;
const int NUM_LEDS = 8;
CRGB leds[NUM_LEDS];
const int defaultBrightness = 96;
const int FRAMES_PER_SECOND = 120;

// Change these pin numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder knobHue(4, 5);
Encoder knobBri(6, 7);
Encoder knobVal(8, 9);
//   avoid using pins with LEDs attached

uint8_t hue = 127;
uint8_t bri = 127;
uint8_t val = 127;
long hueRaw, briRaw, valRaw;
long hueLast=0, briLast=0, valLast=0;

void updateKnobs();
void updateLEDs();
void debugPrint();

Scheduler runner;
// Task t1(50, TASK_FOREVER, &updateKnobs);
Task t1(50, TASK_FOREVER, &updateLEDs);
Task t2(100, TASK_FOREVER, &debugPrint);

void setup() {
    delay(100);
    // while(!Serial); // leonardo
    Serial.begin(115200);
    delay(3000);
    Serial.println("ilovelamp-encodertest1:");

    FastLED.addLeds<WS2812,led1Pin>(leds, NUM_LEDS); //.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(defaultBrightness);

    runner.init();
    runner.addTask(t1);
    runner.addTask(t2);
    t1.enable();
    t2.enable();
}
bool shouldDebugPrint = false;

void debugPrint() {
    shouldDebugPrint = true;
}

void doDebugPrint() {
    if( doDebugPrint ) {
        Serial.print(" Hue:"); Serial.print(hue);
        Serial.print(" Bri:"); Serial.print(bri);
        Serial.print(" Val:"); Serial.print(val);
        Serial.println();
        shouldDebugPrint = false;
    }
}

void updateLEDs() {
    FastLED.setBrightness( bri * (256/32) );

    fadeToBlackBy( leds, NUM_LEDS, 5);

    // int ledpos = map(val, 0,255, 0,NUM_LEDS-1);
    int ledpos = val;
    fill_solid(leds, ledpos, CHSV(hue, 255,255));
    FastLED.show();
}

// this only runs every N milliseconds
void updateKnobs() {

    long ltmp; int delta;

    ltmp = hueRaw = knobHue.read();
    delta = ltmp - hueLast;
    hueLast = hueRaw;
    int hueNew = hue + delta;
    hue = (hueNew<0) ? 0 : (hueNew>255) ? 255 : hueNew;

    ltmp = briRaw = knobBri.read();
    delta = ltmp - briLast;
    briLast = briRaw;
    int briNew = bri + delta;
    bri = (briNew<0) ? 0 : (briNew>255) ? 255 : briNew;

    ltmp = valRaw = knobVal.read();
    delta = ltmp - valLast;
    valLast = valRaw;
    int valNew = val + delta;
    val = (valNew<0) ? 0 : (valNew>255) ? 255 : valNew;

}

//
void loop() {

    updateKnobs(); // should execute as fast as possible

    doDebugPrint();

    runner.execute();
}
