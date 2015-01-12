A fork of Adafruit's Sharp Memory LCD library:
https://github.com/adafruit/Adafruit_SHARP_Memory_Display

This version was modified to works on both AVR-based devices like the Uno (using pseudo-SPI commands) and ARM-based devices like the Due (using hardware SPI). I just made a few changes to how the bytes are sent out. All original credit goes to Adafruit.