/* Encoder Library - TwoKnobs Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 */

#include <Encoder.h>

// Change these pin numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder knobBri(4, 6);
Encoder knobHue(7, 9);
//   avoid using pins with LEDs attached

uint8_t hue = 127;
uint8_t bri = 127;

void setup() {
    delay(3000);
    // while(!Serial); // leonardo
    Serial.begin(115200);
    Serial.println("ilovelamp-encodertest0:");
    // use pin 8 as fake ground for encoder knobHue
    pinMode( 8, OUTPUT);
    digitalWrite( 8, LOW);
    pinMode( 5, OUTPUT);
    digitalWrite( 5, LOW);

    knobHue.write( hue ); // set known state
    knobBri.write( bri ); // set known state
}

void loop() {
    long ltmp;
    ltmp = knobHue.read();
    uint8_t newHue = ltmp % 256; // wrap it around
    knobHue.write(newHue); // keep encoder within our range

    ltmp = knobBri.read();
    uint8_t newBri = ltmp % 256; // wrap it around
    knobBri.write(newBri); // keep encoder within our range
    // BUT THIS MAKES IT GO from 3,2,1,0,255,254 YUCK

    // ltmp = ltmp % 512;
    // knobBri.write(ltmp);
    // uint8_t newBri = (ltmp<256) ? ltmp : 256-ltmp ;
    // delay(80);
    // Serial.print(ltmp); Serial.print(" - "); Serial.println(newBri);

    if (newHue != hue || newBri != bri) {
        Serial.print(" Hue = "); Serial.print(newHue);
        Serial.print(" Bri = "); Serial.print(newBri);
        Serial.println();
        hue = newHue;
        bri = newBri;
    }

    // if a character is sent from the serial monitor,
    // reset both back to zero.
    if (Serial.available()) {
        Serial.read();
        Serial.println("Reset both knobs");
        knobHue.write(127);
        knobBri.write(127);
    }
}
