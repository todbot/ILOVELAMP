#include <Bounce2.h>
#include <Adafruit_NeoPixel.h>
#define FASTLED_INTERNAL
#include "FastLED.h"

const int ledPin  = 10;
const int led2Pin = 16;
const int potAPin = A0;
const int potBPin = A1;
const int potCPin = A2;
const int button1Pin = 15;

bool colorEnabled = true;

#define NUM_LEDS 74
#define NUM_LEDS2 (74/2)   // the rgbw strip

#define BRIGHTNESS_INITIAL 64

uint8_t brightness = BRIGHTNESS_INITIAL;
uint8_t hue = 127;  // 0-255
uint8_t bri = 63;  //  0-63
uint8_t val = NUM_LEDS/4; // 0-NUM_LEDS-1

CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];

Bounce debouncer1 = Bounce();

uint8_t gMode = 0; // Index number of which mode is current
uint8_t gHue;
uint8_t gWhite;

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
void lampMode();
void rainbow();
void sinelon();
void juggle();
void bpm();
SimplePatternList gModes = { lampMode, rainbow, sinelon, juggle, bpm };

// Note: above line & below line mean we're using double the memory for LEDs
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS2, led2Pin, NEO_GRBW + NEO_KHZ800);


void setup()
{
    Serial.begin(115200);

    pinMode( button1Pin, INPUT_PULLUP);
    debouncer1.attach(button1Pin);
    debouncer1.interval(50); // interval in ms

    FastLED.addLeds<WS2812, ledPin>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);

    strip.setBrightness(brightness);
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

    Serial.println("RGBWstrandtest-fastled");

    fill_solid(leds, NUM_LEDS, CRGB::White);
    copyPixels();
}

void loop()
{
    EVERY_N_MILLISECONDS( 100 ) { debugPrint(); }
    // EVERY_N_MILLISECONDS( 50 ) {
    //     gHue++;  // slowly cycle the "base color" through the rainbow
    //     uint32_t c = strip.getPixelColor(1);
    //     Serial.print("gW:"); Serial.print(gWhite);
    //     Serial.print("  rgbw:");
    //     Serial.print(red(c)); Serial.print(",");
    //     Serial.print(green(c)); Serial.print(",");
    //     Serial.print(blue(c)); Serial.print(",");
    //     Serial.print(white(c)); Serial.print("\n");
    // }

    updateButtons();
    updateKnobs();

    gModes[ gMode ]();

    // put any pixel changes here
    EVERY_N_MILLISECONDS( 20 )  {
        // Call the current pattern function once, updating the 'leds' array
        // blurit();
    }

    FastLED.show();  // WWA 3-chan stirp
    FastLED.setBrightness(bri);

    copyPixels(); // to set up RGBW array
    strip.setBrightness(brightness);
    strip.show();    // RGBW 4-chan strip

    yield(); // give USB subsystem some cycles
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

void blurit()
{
    // uint8_t blurAmount = 64; //128; //dim8_raw( beatsin8(3,64, 192) );
    uint8_t blurAmount = 128;
    blur1d( leds, NUM_LEDS, blurAmount);  // blur things a bit
}

//uint8_t makeWhite(CRGB cf, uint8_t wh)
uint8_t makeWhite(CRGB cf)
{
    uint8_t bri = 250;
    if( 0 ) {
        CHSV ch = rgb2hsv_approximate( cf );
        bri = ch.v;
    }
    if( 1 ) {
        uint8_t h,s,v;
        rgb2hsv(cf.r, cf.g, cf.b, h,s,v);
        bri = v;
    }
    if( gWhite > 0 ) {
        bri = gWhite;
    }
    return bri;
}

void copyPixels()
{
    if( !colorEnabled ) return;

    for( uint16_t i=0; i<strip.numPixels(); i++) {
        CRGB cf = leds[i];
        uint8_t bri = makeWhite(cf);
        uint32_t c = strip.Color(cf.r,cf.g,cf.b, bri);
        strip.setPixelColor(i,c);
    }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gMode = (gMode + 1) % ARRAY_SIZE(gModes);
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
    ltmp = analogRead(potAPin);
    hue = ltmp  / 4;
    ltmp = analogRead(potBPin);
    bri = ltmp / 4;
    ltmp = analogRead(potCPin);
    val = map( ltmp, 0,1023, 0, NUM_LEDS);
}

void lampMode()
{
    int ledpos = val; //constrain(val, 0,NUM_LEDS-1);

    fadeToBlackBy(leds, NUM_LEDS, 5);
    CRGB c = CHSV(hue,255,255);

    // if( hue == 255 ) {
    //     Serial.println("full on!");
    //     c = CRGB(255,255,255);
    // }

    int n = NUM_LEDS - ledpos;
    int v = ledpos;
    fill_solid(leds+v, n, c);

    uint8_t blurAmount = 128; //dim8_raw( beatsin8(3,64, 192) );
    blur1d( leds, NUM_LEDS, blurAmount);  // blur things a bit

}

//
//
//

uint8_t rpos = 0;
void singlepixel()
{
    fadeToBlackBy( leds, NUM_LEDS, 2);
    EVERY_N_MILLISECONDS(1000) {
        rpos = random16(NUM_LEDS);
//        leds[ rpos ] = CRGB::White;
        leds[rpos] = CHSV(gHue + random8(64), 255, 255);
    }
}

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( leds, NUM_LEDS, 20);
    byte dothue = 0;
    for( int i = 0; i < 8; i++) {
        leds[ beatsin16(i+3,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
        dothue += 32;
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
void rainbow()
{
    fill_rainbow( leds, NUM_LEDS, gHue, 7); // FastLED's built-in rainbow gen
}

void whitebow()
{
    fill_rainbow( leds, NUM_LEDS, gHue, 255/NUM_LEDS); // FastLED's built-in rainbow gen
//    fill_rainbow( leds, NUM_LEDS, 0, 255/NUM_LEDS); // FastLED's built-in rainbow gen
    EVERY_N_MILLISECONDS(20) {
        gWhite = beatsin8( 20, 1, 255);
    }
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



uint8_t red(uint32_t c)   {  return (c >> 8); }
uint8_t green(uint32_t c) {  return (c >> 16); }
uint8_t blue(uint32_t c)  {  return (c); }
uint8_t white(uint32_t c)  {  return (c>> 24); }


// https://gist.github.com/yoggy/8999625
#define min_f(a, b, c)  (fminf(a, fminf(b, c)))
#define max_f(a, b, c)  (fmaxf(a, fmaxf(b, c)))
void rgb2hsv(const unsigned char &src_r, const unsigned char &src_g, const unsigned char &src_b,
                unsigned char &dst_h, unsigned char &dst_s, unsigned char &dst_v)
{
    float r = src_r / 255.0f;
    float g = src_g / 255.0f;
    float b = src_b / 255.0f;

    float h, s, v; // h:0-360.0, s:0.0-1.0, v:0.0-1.0

    float max = max_f(r, g, b);
    float min = min_f(r, g, b);

    v = max;

    if (max == 0.0f) {
        s = 0; h = 0;
    }
    else if (max - min == 0.0f) {
        s = 0; h = 0;
    }
    else {
        s = (max - min) / max;
        if (max == r) {
            h = 60 * ((g - b) / (max - min)) + 0;
        }
        else if (max == g) {
            h = 60 * ((b - r) / (max - min)) + 120;
        }
        else {
            h = 60 * ((r - g) / (max - min)) + 240;
        }
    }
    if (h < 0) h += 360.0f;

    // dst_h = (unsigned char)(h / 2);   // dst_h : 0-180
    dst_h = (unsigned char)(h * 255);   // dst_h : 0-180
    dst_s = (unsigned char)(s * 255); // dst_s : 0-255
    dst_v = (unsigned char)(v * 255); // dst_v : 0-255
}
