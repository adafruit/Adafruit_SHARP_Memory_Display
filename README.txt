A fork of Adafruit's Sharp Memory LCD library:
https://github.com/adafruit/Adafruit_SHARP_Memory_Display

This version was modified to work on both AVR-based devices like the Uno (using pseudo-SPI bit banging) and ARM-based devices like the Due (using hardware SPI). I just made a few changes to how the bytes are sent out. All original credit goes to Adafruit.

I'm not sure why we use software pseudo-SPI on AVR-based devices. Adafruit wrote that code.

Why do we use hardware SPI on the Due?
1. For some reason Adafruit's pseudo-SPI clock signal is never generated on the Due.
2. Hardware SPI offloads compute cycles from the CPU, to the dedicated hardware SPI peripheral, which is most likely more power efficient.
3. Future iterations of this code can use DMA to further accelerate the transfer process, minimizing awake/compute time. This method lets the microcontroller go back to sleep faster, thus saving power.