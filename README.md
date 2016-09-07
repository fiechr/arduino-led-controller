Arduino Hyperion/Boblight LED-Controller
========================================

Arduino sketch to control RGB-LED strips (e.g. WS2812, WS2811) with serial data received from a Boblight server running e.g. on OpenELEC/Kodi.

I'm using this with a realtively short WS2812b strip (30 LEDs), a Arduino Nano (ATmega328p) and Hyperion server/client running on OpenELEC (5.x) using Kodi (former XBMC) - and it works fine so far.

You'll need the [FastLED library](https://github.com/FastLED/FastLED) for Arduino. FastLED needs to be configured in the sketch for the correct LED type and color order (see FastLED documentation for this).

Also Hyperion or Boblight are needed to be configured separately.

For more (and much better) information, I recommend reading this really excellent article on Tweaking4All.com:
http://www.tweaking4all.com/home-theatre/xbmc/xbmc-boblight-openelec-ws2811-ws2812/

Powersaving
-----------

This function puts the Arduino's ATmega to sleep after some idle time. This should save some power for setups where the USB standby power can't/shouldn't be switched off and you don't want to cut the power for the system entirely.

For this, you also have to modify the circuitry: There needs to be a connection between RX (pin #0) and on of the interrupt pins (I used pin #2 and a 220 ohm resistor). This way the Arduino will wake up as soon as it receives serial data again. Also a SMD capacitor that connects the reset line to the USB-to-Serial chip needs to be removed for this setup to make sense. Otherwise the Arduino will reset instead of just waking up (which is some kind of wakeup, too - I guess - but I didn't tested that).

You can find more info on this topic here:
http://donalmorrissey.blogspot.de/2010/04/sleeping-arduino-part-3-wake-up-via.html

If you don't need this feature, just stick with the normal version.
