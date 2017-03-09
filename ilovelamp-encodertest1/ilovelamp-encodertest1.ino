/* Encoder Library - TwoKnobs Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 */

#include <Encoder.h>
//#include <TaskScheduler.h>
// FASTLED_INTERNAL turns off pragmas that look like warnings
#define FASTLED_INTERNAL
#include "FastLED.h"

const int led1Pin = 2;
const int NUM_LEDS = 38;
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

uint8_t hue = 127;  // 0-255
uint8_t bri = 60;  //  0-63
uint8_t val = NUM_LEDS/2; // 0-NUM_LEDS-1

long hueRaw, briRaw, valRaw;
long hueLast=0, briLast=0, valLast=0;

void updateKnobs();
void updateLEDs();
void debugPrint();

// Scheduler runner;
// Task t1(50, TASK_FOREVER, &updateLEDs);
// Task t2(100, TASK_FOREVER, &debugPrint);

void setup() {
    // while(!Serial); // leonardo
    Serial.begin(115200);
    delay(3000);
    Serial.println("ilovelamp-encodertest1:");

    FastLED.addLeds<WS2812,led1Pin>(leds, NUM_LEDS); //.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(defaultBrightness);

    // runner.init();
    // runner.addTask(t1);
    // runner.addTask(t2);
    // t1.enable();
    // t2.enable();
    Serial.println("here");
}

//bool shouldDebugPrint = false;
// void debugPrint() {
//     shouldDebugPrint = true;
// }

void debugPrint() {
    // if( shouldDebugPrint ) {
        Serial.print(" Hue:"); Serial.print(hue);
        Serial.print(" Bri:"); Serial.print(bri);
        Serial.print(" Val:"); Serial.print(val);
        Serial.println();
        // shouldDebugPrint = false;
    // }
}

//
void updateLEDs() {
    // FastLED.setBrightness( bri * (256/32) );
    FastLED.setBrightness( bri );

    fadeToBlackBy( leds, NUM_LEDS, 25);

    int ledpos = val; //map(val, 0,255, 0,NUM_LEDS-1);
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
    hue = constrain( hueNew, 0, 255);

    ltmp = briRaw = knobBri.read();
    delta = ltmp - briLast;
    briLast = briRaw;
    int briNew = bri + delta;
    bri = constrain( briNew, 0, 64);
    knobBri.write(bri);

    ltmp = valRaw = knobVal.read();
    delta = ltmp - valLast;
    valLast = valRaw;
    int valNew = val + delta;
    val = constrain( valNew, 0, NUM_LEDS-1);
    knobVal.write(val);

}

int debugMillis = 300;
//
void loop() {

    updateKnobs(); // should execute as fast as possible

    EVERY_N_MILLISECONDS( debugMillis ) { debugPrint(); }
    EVERY_N_MILLISECONDS( 20 ) { updateLEDs(); }

    if( Serial.available() ) {
        Serial.println("hello!");
    }

    yield(); // allow USB subsystem some cycles

}
