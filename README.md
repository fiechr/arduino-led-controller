Arduino Hyperion/Boblight LED-Controller
========================================
Arduino sketch to control three wire RGB-LED strips (e.g. WS2812, WS2811) with serial data received from a Boblight server running e.g. on OpenELEC/Kodi.

I'm using this with a short WS2812b strip (30 LEDs), a Arduino Nano (ATmega328p) and Hyperion server/client running on LibreELEC (7.x) using Kodi (former XBMC) - and it works fine so far.

You'll need the [FastLED library](https://github.com/FastLED/FastLED) for Arduino.

Also Hyperion or Boblight are needed to be configured separately.

For more (and much better) information what this is all about, I recommend reading this really excellent article on Tweaking4All.com:
http://www.tweaking4all.com/home-theatre/xbmc/xbmc-boblight-openelec-ws2811-ws2812/

Hyperion configuration
----------------------
The device configuration in `hyperion.config.json` should look something like this:
```
"device" :
{
	"name"       : "adaino",
	"type"       : "adalight",
	"output"     : "/dev/ttyUSB0",
	"rate"     : 115200,
	"delayAfterConnect"     : 0,
	"colorOrder" : "rgb"
}
```
* The `name` doesn't really matter	
* Select `adalight` for `type`.
* `output` needs to match the (USB) port the Arduino is connected to
* `Rate` has to match the `SERIAL_RATE` setting in the sketch.
* `colorOrder` may be set here *or* in the sketch (`COLOR_ORDER`).
* There is no need to set a prefix (the sketch will derive it from the number of LEDs).

Boblight configuration
----------------------
The device configuration in the `boblight.conf` should look similar to this:
```
[device]
name            adaino
type            momo
output          /dev/ttyUSB0
channels        90
prefix          41 64 61 00 1D 48
interval        20000
rate            115200
debug           off
delayafteropen  1000000
```
* The `name` doesn't really matter
* `type` should be set to `momo`
* `output` needs to match the (USB) port the Arduino is connected to
* The value of `channels` is three times the number of the LEDs (30 LEDs --> 90 channels).
* The `prefix` needs to be set according to the provided script/list. Another way would be to set a static prefix in the sketch and use the same one here.

Powersaving
-----------
This function puts the Arduino's ATmega328 to sleep after some idle time. This should save some power for setups where the USB standby power can't/shouldn't be switched off and you don't want to cut the power for the system entirely.

For this, you also have to modify the circuitry: There needs to be a connection between RX (pin #0) and on of the interrupt pins (I used pin #2 and a 220 ohm resistor). This way the Arduino will wake up as soon as it receives serial data again. Also a SMD capacitor that connects the reset line to the USB-to-Serial chip needs to be removed for this setup to make sense. Otherwise the Arduino will reset instead of just waking up (which is some kind of wakeup, too - I guess - but I didn't tested that).

You can find some more info on this topic here:
http://donalmorrissey.blogspot.de/2010/04/sleeping-arduino-part-3-wake-up-via.html
