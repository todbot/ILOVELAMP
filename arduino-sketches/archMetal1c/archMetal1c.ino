/**
 * archMetal1c -- archMetal1 lamp with pot board
 * 
 */
 
// FASTLED_INTERNAL turns off pragmas that look like warnings
#define FASTLED_INTERNAL
#include "FastLED.h"
FASTLED_USING_NAMESPACE

// pin defines
const int ledPin   = 10;  // 2 on rotary-encoder board
const int potApin = A0;
const int potBpin = A1;
const int potCpin = A2;



#define BRIGHTNESS          64
#define FRAMES_PER_SECOND  120

#define NUM_LEDS    68 // lamp2 - arch
CRGB leds[NUM_LEDS];

int brightness = BRIGHTNESS;

uint8_t hue = 127;  // 0-255
uint8_t bri = 63;  //  0-63
uint8_t val = NUM_LEDS/4; // 0-NUM_LEDS-1

uint8_t gMode = 0; // Index number of which mode is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
void raindown();
void rainbow();
void sinelon();
void juggle();
void bpm();

//SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
//SimplePatternList gPatterns = { todrand, rainbow, sinelon, juggle, bpm };
//SimplePatternList gPatterns = { rainbow, sinelon, juggle, bpm };
SimplePatternList gModes = { raindown, rainbow, sinelon, juggle, bpm };

void setup()
{
    delay(2000);
    // while(!Serial);
    Serial.begin(115200);
    delay(1000); // 3 second delay for recovery
    Serial.println("archMetal2");

    FastLED.addLeds<WS2812, ledPin>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
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
        uint8_t blurAmount = 64; //dim8_raw( beatsin8(3,64, 192) );
        blur1d( leds, NUM_LEDS, blurAmount);  // blur things a bit
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
    // debouncer1.update();
    // if( debouncer1.fell() ) {
    //     Serial.println("Button1");
    //     nextPattern();
    // }
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
    val = map( ltmp, 0,1023, 0, NUM_LEDS-1);

    if( ltmp > 1000 ) {
        if( ((millis() - lastValChange) > 10*1000) )  {
            lastValChange = millis();
            nextPattern();
        }
    }
    else {
        gMode = 0;
    }

}


// from NUM_LEDS/2 - ledpos  to NUM_LEDS/2 + ledpos
// => fill_solid(leds+NUM_LEDS/2-ledpos)
void raindown()
{
    int ledpos = constrain(val, 0,NUM_LEDS-1);
    fadeToBlackBy(leds, NUM_LEDS, 5);
    CRGB c = CHSV(hue,255,255);
    if( hue == 0 ) {
        c = CRGB(255,255,255);
    }
    int v = (NUM_LEDS/2) - ledpos;
    int b = 0; // beatsin16(12, 0, 5);
    int n = (ledpos*2);// + b;
    v = v-b;
    v = constrain(v,0,NUM_LEDS-1);
    n = constrain(n,0,NUM_LEDS-1);
    // Serial.print("v:"); Serial.println(v);
    fill_solid(leds+v, n, c);

    uint8_t blurAmount = 64; //dim8_raw( beatsin8(3,64, 192) );
    blur1d( leds, NUM_LEDS, blurAmount);  // blur things a bit

    // glitchy
    // addGlitterColor(10, CRGB(10,10,10));

    // cheezy
    // int pos = beatsin16(13,0,NUM_LEDS);
    // leds[pos] += CHSV( gHue, 255, 92);

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
    int pos = beatsin16( hue,0,NUM_LEDS);
    // int pos = beatsin16(10,0,NUM_LEDS);
    leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = hue; //30; //62;
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
