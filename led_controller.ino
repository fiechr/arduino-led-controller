/* Arduino to Hyperion/Boblight Bridge using FastLED library
**
** Created by Christian Fiebig (neo2001)
** Basic structure partially based on t4a_boblight by Hans Luijten, www.tweaking4all.com
**
** THIS SOFTWARE COMES WITH ABSOLUTELY NO WARRANTY. USE AT YOUR OWN RISK.
*/

#include <avr/sleep.h>  // Allows to control the sleep modes (already included with Arduino)
#include <FastLED.h>  // FastLED lib, see: http://fastled.io/


// Powersaving settings:

const uint8_t PIN_SIGNAL = LED_BUILTIN;  // Optional LED for signaling power saving state
const uint8_t PIN_WAKE_IRQ = 2;  // *Pin* used for wakeup signal
const uint8_t NUM_WAKE_IRQ = 0;  // Corresponding *interrupt* used for wakeup;
                                 // see http://playground.arduino.cc/Code/Interrupts
const uint32_t NUM_WAIT_LOOPS = 10000000;  // Number of loops to wait before going to sleep
                                           // (1M equals approx. 6s, 10M ~ 60s)

// Intro settings:

const uint8_t INTRO_SPEED = 80;  // Speed in % (1-99) for intro/test animation (0 or 100 means off)
                                 // The intro is shown only once after powerup or reset
const uint32_t INTRO_COLORS[] = { 0xFF0000, 0x00FF00, 0x0000FF };  // Colors used in intro/test


// RGB-LED strip settings:

const uint8_t PIN_DATA = 8;  // Pin connected to the data line of the strip
const uint16_t NUM_LEDS = 30;  // Total number of RGB-LEDs to control (has to match Hyperion/Boblight configuration)
const uint16_t SHOW_DELAY = 200;  // Delay after FastLED.show() in µs (1 µs = 0.001 ms)
const uint8_t BRIGHTNESS = 255;  // Max. overall brightness (0-255), can also be controlled by Hyperion
//const uint32_t COLOR_CORRECTION = 0xFFB0F0;  // Color correction function of the FastLED library (not tested!)


// Serial/USB data settings:

const uint32_t SERIAL_RATE = 115200;  // Serial port speed (i.e. 9600, 19200, 38400, 56000, 57600, 115200);
                                      // Has to match the value set for "rate" in hyperion.config.json or boblight.conf
const uint16_t READ_BYTES = (NUM_LEDS * 3);  // Number of bytes of color data to read (3 Bytes/LED)

// Build Adalight compatible 6 byte long prefix:
// - First 3 bytes are static and always the same: 'A', 'd', 'a'
// - Next 2 bytes are dependent on total number of LEDs set above (low and high byte of the 16-bit integer value)
// - Last byte is just a checksum of the previous 2 bytes
const uint8_t _PREFIX_HIGH_BYTE = ((NUM_LEDS - 1) >> 8) & 0xFF;  // Low byte
const uint8_t _PREFIX_LOW_BYTE = (NUM_LEDS - 1) & 0xFF;  // High byte
const uint8_t _PREFIX_CHECKSUM = _PREFIX_HIGH_BYTE ^_PREFIX_LOW_BYTE ^ 0x55;  // Checksum

const uint8_t PREFIX[] = { 'A', 'd', 'a', _PREFIX_HIGH_BYTE, _PREFIX_LOW_BYTE, _PREFIX_CHECKSUM };
const uint8_t PREFIX_SIZE = sizeof(PREFIX) / sizeof(uint8_t);  // Save size of prefix array for convenience
// The built prefix for 30 LEDs should look like this:
// 0x41, 0x64, 0x61, 0x00, 0x1d, 0x48


// Possible values for "state" variable:

enum State : uint8_t { WAITING, DO_PREFIX, DO_DATA };


// Variables:

uint32_t idleCounter = 0;  // Simple counter used for roughly timing the controller's sleep mode
uint8_t buffer[PREFIX_SIZE];  // Buffer for received prefix data
uint8_t state;  // Stores current program state (waiting, processing prefix, processing data)
CRGB strip[NUM_LEDS];  // Contains the current color values for each RGB-LED on the strip


void setup() {

  pinMode(PIN_SIGNAL, OUTPUT);
  digitalWrite(PIN_SIGNAL, HIGH);  // Turn on signal LED

  pinMode(PIN_WAKE_IRQ, INPUT);
  digitalWrite(PIN_WAKE_IRQ, HIGH);  // Activate the internal pull-up resistor

  FastLED.addLeds<WS2812B, PIN_DATA, GRB>(strip, NUM_LEDS);  // See FastLED documentation for this!
  //FastLED.setCorrection(COLOR_CORRECTION);  // Optional overall color correction
  FastLED.setBrightness(BRIGHTNESS);  // Set overall brightness to use
  FastLED.clear();

  showIntro(INTRO_SPEED);

  Serial.begin(SERIAL_RATE);

  state = WAITING;  // Enter endless loop in waiting state

}


void loop() {

  switch(state) {

    case WAITING:  // Wait for prefix (check for a matching first byte)

      if (Serial.available() > 0) {
        if (Serial.read() == PREFIX[0]) {  // If first char matches first prefix char...
          state = DO_PREFIX;  // ...switch to processing prefix
          idleCounter = 0;  // Reset idle counter
          break;
        }
      }

      if (++idleCounter > NUM_WAIT_LOOPS) {  // increment and check idle counter
        idleCounter = 0; // Reset idle counter
        goToSleep();  // after <NUM_WAIT_LOOPS> loops without matching byte received, go to sleep
      }

      break;


    case DO_PREFIX:  // Process prefix (check for a complete match)

      if (Serial.available() > (PREFIX_SIZE - 2)) {  // First byte already checked before, now wait for the rest...

        Serial.readBytes(buffer, PREFIX_SIZE - 1);  // ...and put them into the buffer

        for (uint8_t i = 0; i < (PREFIX_SIZE - 1); i++) {  // Now try to match *each* of them...

          if (buffer[i] == PREFIX[i + 1]) {  // ...against the given prefix; if they are still matching...
            state = DO_DATA;  // ...prepare to switch state for processing the color data
          } else {
            state = WAITING;  // BUT as soon as one byte doesn't match - return to waiting state
            break;  // Don't wait for finishing the for-loop
          }

        }

      }

      break;


    case DO_DATA:  // Process color data

      if (Serial.available() > 0) {  // "0" because Arduino's Serial buffer size is only 64 bytes - so simply just try...
        if ((Serial.readBytes((uint8_t*) strip, READ_BYTES)) == READ_BYTES) {  // Copy all color values at once to strip
          FastLED.show();  // If sucessfull, show them
          delayMicroseconds(SHOW_DELAY);  // Insert a very short delay (µs!)
        }
        state = WAITING;  // Back to waiting for next set of data
      }

      break;

  }

}


// The following function will show a short animation independet of any serial connection or data.
// If you don't want it to show, set INTRO_SPEED to 0 or 100 or remove the function call in setup().
// If you want to keep the program size as small as possible you can remove it altogether.

void showIntro(const uint8_t speed) {

  if (speed == 0) return;
  if (speed == 100) return;  // if no speed given - return instantly (don't show animation)

  // Speed: 0-100, Delay: 0-1000 (ms)
  const uint16_t show_delay = (uint16_t) map(constrain(speed, 0, 100), 0, 100, 1000, 0);  // map speed value to actual delay value

  for (uint8_t i = 0; i < (sizeof(INTRO_COLORS) / sizeof(uint32_t)); i++) {  // for each selected color

    for (uint16_t j = 0; j < (NUM_LEDS / 2); j++) {

      strip[j] = INTRO_COLORS[i];
      strip[(NUM_LEDS - 1 - j)] = INTRO_COLORS[i];
      FastLED.show();
      FastLED.delay(show_delay);

    }

    for (uint16_t j = 0; j < (NUM_LEDS / 2); j++) {

      strip[j] = CRGB::Black;
      strip[(NUM_LEDS - 1 - j)] = CRGB::Black;
      FastLED.show();
      FastLED.delay(show_delay / 2);

    }

  }

}


// Prepares and executes sleep mode.
// To wake up again, an interrupt is used. To make this work, pin 0 (RX) and pin 2 have to be connected
// by using a 220 Ohm resistor. In this case it makes sense to disable the auto reset on the Arduino.
//
// BUT: If auto reset is still enabled (default) the Arduino will reset after Hyperion (re)connects, which in turn
// will wake up the controller anyway. It's kind of a more "dirty" way, but should work (not tested).
// This way no modifictaions should be necessary at all.

void goToSleep() {

  sleep_enable();  // Enable sleep mode settings
  attachInterrupt(NUM_WAKE_IRQ, onInterrupt, LOW);  // Set interrupt and attach Interrupt Service Routine (ISR)
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Define which sleep mode to use (already most effective selected)

  digitalWrite(PIN_SIGNAL, LOW);  // Turn LED off to indicate sleep
  sleep_cpu();  // Set sleep enable (SE) bit. This puts the controller to sleep

  // --- Program execution stops here! ---

  digitalWrite(PIN_SIGNAL, HIGH);  // Turn LED on to indicate wake up

}


// ISR (executed on wakeup)

void onInterrupt() {

  sleep_disable();  // Make absolutely sure device can't sleep again after interrupt was fired
  detachInterrupt(NUM_WAKE_IRQ);  // Remove the interrupt. This makes sure no more interrupts are fired

}
