/**
 *  tree1 -- test sketch for tree1 w/ pot control board
 *  Tod E Kurt / todbot.com
 *  
 */

#include <Bounce2.h>
// FASTLED_INTERNAL turns off pragmas that look like warnings
#define FASTLED_INTERNAL
#include "FastLED.h"
FASTLED_USING_NAMESPACE

// pin defines
const int ledPin   = 10;  // 2 on rotary-encoder board
const int led2Pin  = 16; 
const int potApin = A0;
const int potBpin = A1;
const int potCpin = A2;
const int button1pin = 15;


#define NUM_LEDS    104     // helixMetal1
#define BRIGHTNESS          64
#define FRAMES_PER_SECOND  120

CRGB leds[NUM_LEDS];
//CRGB leds2[NUM_LEDS];

Bounce debouncer1 = Bounce();


int brightness = BRIGHTNESS;

uint8_t hue = 127;  // 0-255
uint8_t bri = 63;  //  0-63
uint8_t val = NUM_LEDS/4; // 0-NUM_LEDS-1

uint8_t gMode = 0; // Index number of which mode is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
void lampMode();
void rainbow();
void sinelon();
void juggle();
void bpm();

SimplePatternList gModes = { lampMode, rainbow, sinelon, juggle, bpm };

void setup()
{
    delay(200);
    Serial.begin(115200);
    Serial.println("tree1");

    FastLED.addLeds<WS2812, ledPin>(leds, NUM_LEDS);
    FastLED.addLeds<WS2812, led2Pin>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    pinMode( button1pin, INPUT_PULLUP);
    debouncer1.attach(button1pin);
    debouncer1.interval(50); // interval in ms
    
    Serial.print("doing fadeup: ");
    
    // do fadeup
    updateKnobs(); // sets initial setting
    uint8_t oldbri = bri;
    int fadems = 2000 / (bri+1);
    Serial.print(bri); Serial.print(","); Serial.println(fadems); 
    for( int i=0; i< oldbri; i++ ) { 
        lampMode();
        FastLED.show();     // send the 'leds' array out to the actual LED strip
        FastLED.setBrightness(i);
        delay(fadems);
    }
    Serial.println("setup done");
}

void loop()
{
    updateButtons();
    updateKnobs();

    // Call the current pattern function once, updating the 'leds' array
    gModes[ gMode ]();

    EVERY_N_MILLISECONDS( 20 ) {
        gHue++;  // slowly cycle the "base color" through the rainbow
        if( gMode !=0 ) { 
            uint8_t blurAmount = 128; //dim8_raw( beatsin8(3,64, 192) );
            blur1d( leds, NUM_LEDS, blurAmount);  // blur things a bit
        }
    }

    EVERY_N_MILLISECONDS( 100 ) { debugPrint(); }

    FastLED.setBrightness(bri);
    FastLED.show();     // send the 'leds' array out to the actual LED strip
    yield(); // give USB subsystem some cycles
    // delay(1);
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gMode = (gMode + 1) % ARRAY_SIZE(gModes);
}

uint32_t lastValChange;
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

    ltmp = analogRead(potApin);
    hue = ltmp  / 4;

    ltmp = analogRead(potBpin);
    bri = ltmp / 4;

    ltmp = analogRead(potCpin);
    val = map( ltmp, 0,1023, 0, NUM_LEDS);

    // change mode to demos when knob is in particular position
    
//    if( hue <= 2 ) {
//        if( ((millis() - lastValChange) > 10*1000) )  {
//            lastValChange = millis();
//            nextPattern();
//        }
//    }
//    else {
//        gMode = 0;
//    }

}

CRGB colorCurr;
CRGB colorNew;
fract8 cAmount;

void lampMode()
{
    int ledpos = val; //constrain(val, 0,NUM_LEDS-1);
    
    fadeToBlackBy(leds, NUM_LEDS, 5);
    CRGB c = CHSV(hue,255,255);
    
//    colorNew = c;
    if( hue >= 250 ) {
        Serial.println("full on!");
   // uncommenting this bugs out the thing why?
//        c = CRGB(255,255,255);
    }
    
    int n = NUM_LEDS - ledpos;
    int v = ledpos;
    fill_solid(leds+v, n, c);

     uint8_t blurAmount = 128; //dim8_raw( beatsin8(3,64, 192) );
     blur1d( leds, NUM_LEDS, blurAmount);  // blur things a bit

    // glitchy
    // addGlitterColor(10, CRGB(10,10,10));

    // cheezy
    // int pos = beatsin16(13,0,NUM_LEDS);
    // leds[pos] += CHSV( gHue, 255, 92);

}


void rainbow()
{
    // FastLED's built-in rainbow generator
    fill_rainbow( leds, NUM_LEDS, hue, 7);
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
void addGlitterColor( fract8 chanceOfGlitter, CRGB c)
{
    if( random8() < chanceOfGlitter) {
        leds[ random16(NUM_LEDS) ] += c;
    }
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 20);
//    int pos = beatsin16( hue,0,NUM_LEDS);
     int pos = beatsin16(10,0,NUM_LEDS);
    leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
//    uint8_t BeatsPerMinute = hue; //30; //62;
    uint8_t BeatsPerMinute = 15; //62;
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
    Serial.print(" gMode:"); Serial.print(gMode );
    Serial.print(" Hue:"); Serial.print(hue);
    Serial.print(" Bri:"); Serial.print(bri);
    Serial.print(" Val:"); Serial.print(val);
    Serial.println();
}

void confetti()
{
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV( gHue + random8(64), 200, 255);
}
