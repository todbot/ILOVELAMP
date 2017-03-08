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

void updateKnobs();
void updateLEDs();

Scheduler runner;
// Task t1(50, TASK_FOREVER, &updateKnobs);
Task t1(1000/FRAMES_PER_SECOND, TASK_FOREVER, &updateLEDs);

void setup() {
    // while(!Serial); // leonardo
    Serial.begin(115200);
    Serial.println("ilovelamp-encodertest0:");

    delay(3000);
    FastLED.addLeds<WS2812,led1Pin>(leds, NUM_LEDS); //.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(defaultBrightness);

    knobHue.write( hue ); // set known state
    knobBri.write( bri ); // set known state
    knobVal.write( val ); // set known state

    runner.init();
    runner.addTask(t1);
    t1.enable();
}

uint32_t lastHueMillis, lastBriMillis, lastValMillis;

void updateLEDs() {
    FastLED.setBrightness( bri * (256/32) );

    fadeToBlackBy( leds, NUM_LEDS, 5);

    // int ledpos = (val/2) % NUM_LEDS;
    // int ledposL = (ledpos==0) ? NUM_LEDS-1 : ledpos-1;
    // int ledposR = (ledpos==NUM_LEDS-2) ? 0 : ledpos+1;
    // leds[ ledpos ] = CHSV( hue, 255, 255);
    // leds[ ledposL ] = CHSV( hue, 255, 120);
    // leds[ ledposR ] = CHSV( hue, 255, 120);

    // int ledpos = map(val, 0,255, 0,NUM_LEDS-1);
    int ledpos = val;
    fill_solid(leds, ledpos, CHSV(hue, 255,255));
    FastLED.show();
}

// this only runs every N milliseconds
void updateKnobs() {
    long ltmp;
    ltmp = knobHue.read();
    uint8_t newHue = ltmp % 256; // wrap it around
    knobHue.write(newHue); // keep encoder within our range

    ltmp = knobBri.read();
    uint8_t newBri = ltmp % 32; // wrap it around
    knobBri.write(newBri); // keep encoder within our range
    // BUT THIS MAKES IT GO from 3,2,1,0,255,254 YUCK

    ltmp = knobVal.read();
    uint8_t newVal = (ltmp > NUM_LEDS) ? NUM_LEDS : (ltmp<0) ? 0 : ltmp;
    // uint8_t newVal = (ltmp > NUM_LEDS) ? NUM_LEDS : (ltmp<0) ? 0 : ltmp;
    // uint8_t newVal = ltmp % 256; // wrap it around
    knobVal.write(newVal); // keep encoder within our range

    bool isChanged = false;

    if (newHue != hue ) {
        if( abs(hue - newHue) > 5 ) {
            Serial.println("BIG JUMP");
        }
        hue = newHue;
        isChanged = true;
    }
    if( newBri != bri ) {
        bri = newBri;
        // int deltaBri = (bri - newBri) * 10;
        // bri += deltaBri;
        isChanged = true;
    }
    if( newVal != val) {
        val = newVal;
        isChanged = true;
    }

    if( isChanged ) {
        Serial.print(" Hue:"); Serial.print(newHue);
        Serial.print(" Bri:"); Serial.print(newBri);
        Serial.print(" Val:"); Serial.print(newVal);
        Serial.println();
    }

    // if a character is sent from the serial monitor,
    // reset both back to zero.
    if (Serial.available()) {
        Serial.read();
        Serial.println("Reset knobs");
        knobHue.write(127);
        knobBri.write(127);
        knobVal.write(127);
    }
}

//
void loop() {

    updateKnobs(); // should execute as fast as possible

    runner.execute();
}
