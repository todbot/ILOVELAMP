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
const int NUM_LEDS = 45;
CRGB leds[NUM_LEDS];
const int brightnessDefault = 60;
const int brightnessMax = 250;
const int FRAMES_PER_SECOND = 120;

// Change these pin numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder knobHue(4, 5);
Encoder knobBri(6, 7);
Encoder knobVal(8, 9);
//   avoid using pins with LEDs attached

long knobHueLast, knobBriLast, knobValLast;

uint8_t hue = 127;  // 0-255
uint8_t bri = 63;  //  0-63
uint8_t val = NUM_LEDS/2; // 0-NUM_LEDS-1

//long hueRaw, briRaw, valRaw;
//long hueLast=0, briLast=0, valLast=0;

void updateKnobs();
void updateLEDs();
void debugPrint();


void setup() {
    // while(!Serial); // leonardo
    Serial.begin(115200);
    delay(3000);
    Serial.println("ilovelamp-encodertest2:");

    FastLED.addLeds<WS2812,led1Pin>(leds, NUM_LEDS); //.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(brightnessDefault);

}

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
    CRGB c = CHSV(hue,255,255);
    if( hue == 0 ) { 
        c = CRGB(255,255,255);
    }
    fill_solid(leds+ledpos+1, NUM_LEDS-ledpos, c);

    FastLED.show();
}

// get sign of int
int sign(int x) {
    return (x > 0) - (x < 0);
}

// this only runs every N milliseconds
void updateKnobs() {

    long ltmp; int diff;

    ltmp = knobHue.read();
    diff = ltmp - knobHueLast;
    if( abs(diff) >= 2 ) { 
        knobHueLast = ltmp;
        ltmp = hue;
        ltmp += diff;
        hue = (ltmp < 0) ? 0 : (ltmp > 255) ? 255 : ltmp;
    }
    
    ltmp = knobBri.read();
    diff = ltmp - knobBriLast;
    if( abs(diff) >= 2 ) { 
        knobBriLast = ltmp;
        ltmp = bri;
        ltmp += diff*2 ;
        bri = (ltmp < 0) ? 0 : (ltmp > 255) ? 255 : ltmp;
    }
    ltmp = knobVal.read();
    diff = ltmp - knobValLast;
    if( abs(diff) >= 4 ) { 
        knobValLast = ltmp;
        ltmp = val;
        ltmp += (diff / 4); // pos is every four
        Serial.print(val); 
        Serial.print(", diff:");Serial.print(diff); 
        Serial.print(", ltmp:");Serial.println(ltmp);
        val = (ltmp < 0) ? 0 : (ltmp > NUM_LEDS-1) ? NUM_LEDS-1 : ltmp;
    }
}

int debugMillis = 100;
//
void loop() {

    // EVERY_N_MILLISECONDS( 1 ) { updateKnobs(); } // should execute as fast as possible
    updateKnobs();

    EVERY_N_MILLISECONDS( debugMillis ) { debugPrint(); }

    EVERY_N_MILLISECONDS( 20 ) { updateLEDs(); }

    if( Serial.available() ) {
        Serial.println("hello!");
    }

    yield(); // allow USB subsystem some cycles

}
