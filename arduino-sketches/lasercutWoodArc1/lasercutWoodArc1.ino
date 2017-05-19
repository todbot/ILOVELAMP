
/*
 * For use with "lasercutWoodArc1" lamp and "pot-boardDesp3"
 * Tod Kurt / todbot.com
 * Uses ESP8266 D1 Mini driving single SK6812WWA strip, split into two with a NeoJoint
 */
 
#include <NeoPixelBus.h>
#include <Bounce2.h>

//#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERNAL
#include "FastLED.h"
FASTLED_USING_NAMESPACE

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

const int NUM_LEDS = 33*4; //33 * 4;

const int ledPin = 2;
const int led2Pin = 0;
const int butt1Pin = 14;
const int butt2Pin = 12;
const int butt3Pin = 13;
const int potPin = A0;

CRGB leds[NUM_LEDS];

const int FRAMES_PER_SECOND = 60;

NeoPixelBus<NeoGrbwFeature, NeoEsp8266Uart800KbpsMethod> strip(NUM_LEDS, ledPin);
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();


float potval;
float lastval;
CRGB gColors[] = { 
    CRGB(255,255,255), 
    
    CRGB(  0,255,255),
    CRGB(255,  0,255),
    CRGB(255,255,  0),
    
    CRGB(  0,  0,255),
    CRGB(  0,255,  0),
    CRGB(255,  0,  0)
};
CRGB gColor = gColors[0];
int gColorsPos = 0;
uint8_t gHue;


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
void modeLamp();
void modeRainbow();
void modeBpm();
void modeSinelon();
SimplePatternList gPatternModes = { modeRainbow, modeBpm, modeSinelon };
uint8_t gMode = 0;
uint8_t gPatternMode = 0;

//
void setup()
{
    Serial.begin(115200);
    delay(1);
    
    pinMode(potPin, INPUT);
    pinMode(butt1Pin, INPUT_PULLUP);
    pinMode(butt2Pin, INPUT_PULLUP);
    pinMode(butt3Pin, INPUT_PULLUP);


    debouncer1.attach(butt1Pin);
    debouncer1.interval(50); // interval in ms
    debouncer2.attach(butt2Pin);
    debouncer2.interval(50); // interval in ms
    debouncer3.attach(butt3Pin);
    debouncer3.interval(50); // interval in ms

    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();    
}

//
void loop()
{
    EVERY_N_MILLISECONDS( 20 ) { 

        updateInterface();
        
        if( gMode == 0 ) { 
            modeLamp();
        }
        else { 
            gPatternModes[ gPatternMode ]();
        }
        
//        blur1d( leds, NUM_LEDS, 128);  // blur things a bit
        copyPixels();
       
    }
    
    EVERY_N_MILLISECONDS( 100 ) { 
        debugPrint();
    }
        
    strip.Show();
    delay( 1000 / FRAMES_PER_SECOND );
    
}

void updateInterface()
{
    potval = analogRead( potPin );
        
    debouncer1.update();
    debouncer2.update();
    debouncer3.update();
     
    if( debouncer1.fell() ) {
        gMode = !gMode;
//        gMode = (gMode+1) % ARRAY_SIZE(gModes);
        Serial.print(F("Button1:")); Serial.println(gMode);
    }
    
    if( debouncer2.fell() ) { 
        gColorsPos = (gColorsPos+1) % ARRAY_SIZE(gColors); // wrap around
        gColor = gColors[ gColorsPos ];
        Serial.print(F("Button2:")); Serial.println(gColorsPos);
    }

    if( debouncer3.fell() ) {
        gPatternMode = (gPatternMode +1) % ARRAY_SIZE(gPatternModes);
        Serial.print(F("Button3!")); Serial.println(gPatternMode);        
    }

}

void modeLamp()
{
    int l0 = map( potval, 1023,0, 0,1023); // reverse
    int l1 = map(map(potval, 1023,0, 0,NUM_LEDS), 0,NUM_LEDS, 0,1023); // reverse, scale, reverse,scale
    int dl = l0-l1;  // delta between actual and scaled
    int llbri = map( dl, 0, (1023/NUM_LEDS)+1, 255,0 );
        
    // reverse & scale
    int l = map(potval, 1023,0, 0,NUM_LEDS); // reverse & scale
        
    lastval = smooth(l, lastval);
        
//    fadeToBlackBy(leds, NUM_LEDS, 25);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fill_solid(leds+l, NUM_LEDS-l, gColor);

    // slowly turn on next pixel
    CRGB c2 = gColor;
    c2.subtractFromRGB(255-llbri);
    fill_solid(leds+l-1, 1, c2);
//    CRGB c3 = c2;
//     c3 /= 2; // .subtractFromRGB(255-llbri);
//     fill_solid(leds+l-2, 1, c3);

}

void modeRainbow()
{
    // FastLED's built-in rainbow generator
    fill_rainbow( leds, NUM_LEDS, gHue++, 8);
    for( int i=0; i<NUM_LEDS; i++) {
        leds[i] %= potval/4; // dim some percentage
    }
}

void modeSinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 20);
//    int pos = beatsin16( hue,0,NUM_LEDS);
    int pos = beatsin16(10,0,NUM_LEDS);
//    leds[pos] += CHSV( gHue, 255, potval/4);
    leds[pos] += gColor;
    leds[pos] %= potval/4;
}
void modeBpm()
{
    gHue++;
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 30; //62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
        leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
        leds[i] %= potval/4;
    }
}

void debugPrint()
{
    Serial.print(F("mode:")); Serial.print(gMode); Serial.print(F(":"));
    Serial.print(debouncer1.read()); Serial.print(F(","));
    Serial.print(debouncer2.read()); Serial.print(F(","));
    Serial.print(debouncer3.read()); Serial.print(F(":"));
        
    Serial.print(potval); Serial.print(F(" : "));
//    Serial.print(lastval); Serial.print(F(" : "));
//    Serial.print(dl); Serial.print(F(" : "));
//    Serial.print(llbri); Serial.print(F(" : "));
//    Serial.print(l); 
    Serial.println();
    //  512 - 11.69 - 1,1,1 - 43 - -6 - 11  // negative?    
}


float smooth(float val, float last)
{
    return last + ((val - last) * 0.3);
}

// convert FastLED pixels to whatever we're using (NeoPixelBus in this case)
void copyPixels()
{
    for( int i=0; i<NUM_LEDS; i++) { 
        CRGB c0 = leds[i];
        RgbColor c(c0.r, c0.g, c0.b);
        strip.SetPixelColor(i, c);
    }

}

