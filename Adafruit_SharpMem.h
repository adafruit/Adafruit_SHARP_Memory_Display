/*********************************************************************
This is an Arduino library for our Monochrome SHARP Memory Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1393

These displays use SPI to communicate, 3 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include "Arduino.h"

#if defined(RAMSTART) && defined(RAMEND) && ((RAMEND - RAMSTART) < 4096)
#warning "Display may not work on devices with less than 4K RAM"
#endif

#include <Adafruit_GFX.h>

/**
 * @brief Class to control a Sharp memory display
 *
 */
class Adafruit_SharpMem : public Adafruit_GFX {
public:
  Adafruit_SharpMem(uint8_t clk, uint8_t mosi, uint8_t ss, uint16_t w = 96,
                    uint16_t h = 96);
  boolean begin();
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  uint8_t getPixel(uint16_t x, uint16_t y);
  void clearDisplay();
  void refresh(void);

private:
  uint8_t _ss, _clk, _mosi;
  uint32_t _sharpmem_vcom;

  void sendbyte(uint8_t data);
  void sendbyteLSB(uint8_t data);
};
