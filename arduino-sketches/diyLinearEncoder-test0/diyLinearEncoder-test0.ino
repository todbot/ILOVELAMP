/* 
 * DIY Linear Encoder test0
 * Tod E. Kurt / todbot.com
 */

#include <Encoder.h>
// FASTLED_INTERNAL turns off pragmas that look like warnings
#define FASTLED_INTERNAL
#include <FastLED.h>

#define BRIGHTNESS    255
#define NUM_LEDS      19
CRGB leds[NUM_LEDS];

const int ledPin   = 10;  // 2 on rotary-encoder board

float ledpos = 0; // ledpos ranges from 0-NUM_LEDS-1
long oldPosition  = -999;

Encoder myEnc(15, 16);

void setup()
{
  Serial.begin(115200);
  Serial.println("Basic Encoder Test w/ FastLED:");

  FastLED.addLeds<WS2812, ledPin>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.println("setup done");
  oldPosition = myEnc.read(); // setup
}


void loop()
{
    
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    float diff = oldPosition - newPosition;
    oldPosition = newPosition;
    
    ledpos = ledpos - diff;
    
    ledpos = constrain(ledpos, 0, NUM_LEDS); 

    Serial.print(newPosition); Serial.print(" diff:"); Serial.print(diff);
    Serial.print(" ledpos: "); Serial.print(ledpos);
    Serial.println();
    
    uint8_t hue = 0;
    CRGB c = CRGB(255,0,255);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fill_solid(leds, ledpos, c);

  } // change

  
  FastLED.show();     // send the 'leds' array out to the actual LED strip
  yield(); // give USB subsystem some cycles
 
}
