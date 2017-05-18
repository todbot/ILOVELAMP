
/*
 * For use with "woodArc3" lamp.
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

const int NUM_LEDS = 24;

const int ledPin = 2;
const int led2Pin = 0;
const int butt1Pin = 14;
const int butt2Pin = 12;
const int butt3Pin = 13;
const int potPin = A0;

CRGB leds[NUM_LEDS];

const int FRAMES_PER_SECOND = 60;


NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(NUM_LEDS, ledPin);
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

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
void modeLamp();
void modeRainbow();
//void modeSinelon();
SimplePatternList gModes = { modeLamp, modeRainbow };
uint8_t gMode = 0;

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
    updateInterface();
    
    EVERY_N_MILLISECONDS( 50 ) { 

        // call current mode function, updating leds array
        gModes[ gMode ]();
        
//        blur1d( leds, NUM_LEDS, 128);  // blur things a bit
        copyPixels();
       
    }
        
    strip.Show();
    delay( 1000 / FRAMES_PER_SECOND );
    
}

void modeRainbow()
{
    static uint8_t hue;
    // FastLED's built-in rainbow generator
    fill_rainbow( leds, NUM_LEDS, hue++, 8);
    for( int i=0; i<NUM_LEDS; i++) {
        leds[i] %= potval/4; // dim some percentage
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

void updateInterface()
{
    potval = analogRead( potPin );
        
    debouncer1.update();
    debouncer2.update();
    debouncer3.update();
     
    if( debouncer1.fell() ) {
        gMode = (gMode+1) % ARRAY_SIZE(gModes);
        Serial.print(F("Button1:")); Serial.println(gMode);
    }
    
    if( debouncer2.fell() ) { 
        gColorsPos = (gColorsPos+1) % ARRAY_SIZE(gColors); // wrap around
        gColor = gColors[ gColorsPos ];
        Serial.print(F("Button2:")); Serial.println(gColorsPos);
    }

    if( debouncer3.fell() ) {
        Serial.println(F("Button3!"));
    }

//    EVERY_N_MILLISECONDS(50) { 
//    }

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

