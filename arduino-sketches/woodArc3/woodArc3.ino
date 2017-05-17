
#include <NeoPixelBus.h>
#include <Bounce2.h>

//#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERNAL
#include "FastLED.h"
FASTLED_USING_NAMESPACE

const int ledPin = 2;
const int led2Pin = 0;
const int butt1Pin = 14;
const int butt2Pin = 12;
const int butt3Pin = 13;
const int potPin = A0;

const int NUM_LEDS = 24;
CRGB leds[NUM_LEDS];

#define FRAMES_PER_SECOND 60

int brightness = 95;
uint8_t hue = 0;

uint8_t mode = 0;
const int modeCount = 2;

NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(NUM_LEDS, ledPin);
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();

float potval;
float lastval;
CRGB gColors[] = { CRGB::White, CRGB(0,255,0), CRGB(0,0,255), CRGB(255,0,0) };
CRGB gColor = gColors[0];
int gColorsCount = 4; 
int gColorsPos = 0;


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

void loop()
{
    updateButtons();
    
    potval = analogRead( potPin );
        
    EVERY_N_MILLISECONDS(50) { 
        
        int l0 = map( potval, 1023,0, 0,1023); // reverse
        int l1 = map(map(potval, 1023,0, 0,NUM_LEDS), 0,NUM_LEDS, 0,1023); // reverse, scale, reverse,scale
        int dl = l0-l1;  // delta between actual and scaled
        int llbri = map( dl, 0, (1023/NUM_LEDS)+1, 255,0 );
         
        // reverse & scale
        int l = map(potval, 1023,0, 0,NUM_LEDS); // reverse & scale
        
        lastval = smooth(l, lastval);
        
        Serial.print(potval); Serial.print(" : ");
        Serial.print(lastval); Serial.print(" : ");
        Serial.print(dl); Serial.print(" : ");
        Serial.print(llbri); Serial.print(" : ");
        Serial.print(l); Serial.println();
         //  512 - 11.69 - 1,1,1 - 43 - -6 - 11  // negative?
                    
        CRGB c2 = gColor;
        c2.subtractFromRGB(255-llbri);
        
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        fill_solid(leds+l, NUM_LEDS-l, gColor);
        fill_solid(leds+l-1, 1, c2);
        
        blur1d( leds, NUM_LEDS, 128);  // blur things a bit
        copyPixels();
    }
    
    strip.Show();
    delay( 1000 / FRAMES_PER_SECOND );
    
}

void updateButtons()
{
        
     debouncer1.update();
     debouncer2.update();
     
     if( debouncer1.fell() ) {
        Serial.println("Button1");
        mode++;
        if( mode >= modeCount ) { mode = 0; } 
     }
     if( debouncer2.fell() ) { 
        Serial.println("Button1");
        gColor = gColors[ gColorsPos++ ];
        gColorsPos = gColorsPos % gColorsCount; // wrap around
     }
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

