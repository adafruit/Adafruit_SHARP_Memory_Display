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

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#ifndef __AVR__
#include <SPI.h>
#endif

#include <Adafruit_GFX.h>
#ifdef __AVR
  #include <avr/pgmspace.h>
#elif defined(ESP8266)
  #include <pgmspace.h>
#endif

/*=========================================================================
    Sharp Memorty Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (96x96, 128x128).
    Select the appropriate display below to create an appropriately
    sized framebuffer, etc.

    SHARPMEM_96_96  96x96 pixel display

    SHARPMEM_128_128  128x128 pixel display

    -----------------------------------------------------------------------*/
//   #define SHARPMEM_96_96
   #define SHARPMEM_128_128
/*=========================================================================*/

#if !defined SHARPMEM_96_96 && !defined SHARPMEM_128_128
  #error "At least one SHARP MEMORY display must be specified in Adafruit_SharpMem.h"
#endif

// LCD Dimensions
#if defined SHARPMEM_96_96
  #define SHARPMEM_LCDWIDTH                  (96)
  #define SHARPMEM_LCDHEIGHT                 (96)
#endif
#if defined SHARPMEM_128_128
  #define SHARPMEM_LCDWIDTH                  (128)
  #define SHARPMEM_LCDHEIGHT                 (128)
#endif

class Adafruit_SharpMem : public Adafruit_GFX {
 public:
  Adafruit_SharpMem(uint8_t clk, uint8_t mosi, uint8_t ss);
  void begin(void);
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  uint8_t getPixel(uint16_t x, uint16_t y);
  void clearDisplay();
  void refresh(void);
  void clearDisplayBuffer();
  virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

 private:
  uint8_t _ss, _clk, _mosi;
#ifdef __AVR__
  volatile uint8_t *dataport, *clkport;
  uint8_t _sharpmem_vcom, datapinmask, clkpinmask;
#endif
#if defined(__SAM3X8E__) || defined(ARDUINO_ARCH_SAMD)
    volatile RwReg *dataport, *clkport;
    uint32_t _sharpmem_vcom, datapinmask, clkpinmask;
#endif

  inline void drawFastVLineInternal(int16_t x, int16_t y, int16_t h, uint16_t color) __attribute__((always_inline));
  inline void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) __attribute__((always_inline));

  void sendbyte(uint8_t data);
  void sendbyteLSB(uint8_t data);
};
