
// FASTLED_INTERNAL turns off pragmas that look like warnings
#define FASTLED_INTERNAL
#include "FastLED.h"
FASTLED_USING_NAMESPACE
#include <Encoder.h>
#include <Bounce2.h>

// pin defines
const int ledPin   = 2;

const int enc1APin = 4;
const int enc1BPin = 5;
const int enc2APin = 6;
const int enc2BPin = 7;
const int enc3APin = 8;
const int enc3BPin = 9;

const int button1Pin = 10;
const int button2Pin = 16;

#define BRIGHTNESS          64
#define FRAMES_PER_SECOND  120

#define NUM_LEDS    68 // lamp2 - arch
CRGB leds[NUM_LEDS];

int brightness = BRIGHTNESS;

Encoder knobHue(4, 5);
Encoder knobBri(6, 7);
Encoder knobVal(8, 9);

Bounce debouncer1 = Bounce();

long knobHueLast;
long knobBriLast;
long knobValLast;

uint8_t hue = 127;  // 0-255
uint8_t bri = 63;  //  0-63
uint8_t val = NUM_LEDS/2; // 0-NUM_LEDS-1

uint8_t gPatt = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
void raindown();
void rainbow();
void sinelon();
void juggle();
void bpm();
void lampMode();

//SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
//SimplePatternList gPatterns = { todrand, rainbow, sinelon, juggle, bpm };
//SimplePatternList gPatterns = { rainbow, sinelon, juggle, bpm };
SimplePatternList gPatterns = { raindown, rainbow, sinelon, juggle, bpm };

void setup()
{
    delay(2000);
    // while(!Serial);
    Serial.begin(115200);
    delay(1000); // 3 second delay for recovery
    Serial.println("archMetal1");

    pinMode( button1Pin, INPUT_PULLUP);
    pinMode( button2Pin, INPUT_PULLUP);
    // After setting up the button, setup the Bounce instance :
    debouncer1.attach(button1Pin);
    debouncer1.interval(100); // interval in ms

    FastLED.addLeds<WS2812, ledPin>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    Serial.println("here");
}

void loop()
{
    updateButtons();
    updateKnobs();

    // Call the current pattern function once, updating the 'leds' array
    gPatterns[ gPatt ]();
    // EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically

    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

    EVERY_N_MILLISECONDS( 100 ) { debugPrint(); }

    FastLED.show();     // send the 'leds' array out to the actual LED strip
    yield(); // give USB subsystem some cycles
    // delay(1);
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void nextPattern()
{
   // for( int i=0; i<100; i++) {
   //     fadeToBlackBy( leds, NUM_LEDS, 10);
   //     FastLED.show();
   //     delay(10);
   // }
    // add one to the current pattern number, and wrap around at the end
    gPatt = (gPatt + 1) % ARRAY_SIZE(gPatterns);
}

void updateButtons()
{
    debouncer1.update();
    if( debouncer1.fell() ) {
        Serial.println("Button1");
        nextPattern();
    }
}
// this only runs every N milliseconds
void updateKnobs()
{
    long ltmp; int diff;

    ltmp = knobHue.read();
    diff = ltmp - knobHueLast;
    if( abs(diff) >= 2 ) {
        knobHueLast = ltmp;
        ltmp = hue;
        ltmp += diff*4;
        hue = (ltmp < 0) ? 0 : (ltmp > 255) ? 255 : ltmp;
    }
    ltmp = knobBri.read();
    diff = ltmp - knobBriLast;
    if( abs(diff) >= 2 ) {
        knobBriLast = ltmp;
        ltmp = bri;
        ltmp += diff*8 ;
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


// from NUM_LEDS/2 - ledpos  to NUM_LEDS/2 + ledpos
// => fill_solid(leds+NUM_LEDS/2-ledpos)
void raindown()
{
    int ledpos = constrain(val, 0,NUM_LEDS/2);
    fadeToBlackBy(leds, NUM_LEDS, 5);
    CRGB c = CHSV(hue,255,255);
    if( hue == 0 ) {
        c = CRGB(255,255,255);
    }
    int v = (NUM_LEDS/2) - ledpos;
    // Serial.print("v:"); Serial.println(v);
    fill_solid(leds+(NUM_LEDS/2)-ledpos, ledpos*2, c);
    FastLED.setBrightness(bri);
}

void todrand()
{
    static uint8_t numranded = 0;
    int n = random16(NUM_LEDS);
    leds[ n ] += CRGB(15,15,15);
    numranded++;
    if( numranded >= NUM_LEDS/2 )  {
        fadeToBlackBy( leds, NUM_LEDS, 1);
    }
}

void rainbow()
{
    // FastLED's built-in rainbow generator
    fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
    if( random8() < chanceOfGlitter) {
        leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
}

void confetti()
{
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 20);
    int pos = beatsin16(13,0,NUM_LEDS);
    leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 30; //62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
        leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
    }
}

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 20);
    byte dothue = 0;
    for( int i = 0; i < 8; i++) {
        leds[ beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

//
void debugPrint()
{
    Serial.print(" gPatt:"); Serial.print(gPatt);
    Serial.print(" Hue:"); Serial.print(hue);
    Serial.print(" Bri:"); Serial.print(bri);
    Serial.print(" Val:"); Serial.print(val);
    Serial.println();
}
