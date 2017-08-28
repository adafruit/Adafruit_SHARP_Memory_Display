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

#include "Adafruit_SharpMem.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif
#ifndef _swap_uint16_t
#define _swap_uint16_t(a, b) { uint16_t t = a; a = b; b = t; }
#endif

#define BLACK 0
#define WHITE 1
#define INVERSE 2

/**************************************************************************
    Sharp Memory Display Connector
    -----------------------------------------------------------------------
    Pin   Function        Notes
    ===   ==============  ===============================
      1   VIN             3.3-5.0V (into LDO supply)
      2   3V3             3.3V out
      3   GND
      4   SCLK            Serial Clock
      5   MOSI            Serial Data Input
      6   CS              Serial Chip Select
      9   EXTMODE         COM Inversion Select (Low = SW clock/serial)
      7   EXTCOMIN        External COM Inversion Signal
      8   DISP            Display On(High)/Off(Low)

 **************************************************************************/

#define SHARPMEM_BIT_WRITECMD   (0x80)
#define SHARPMEM_BIT_VCOM       (0x40)
#define SHARPMEM_BIT_CLEAR      (0x20)
#define TOGGLE_VCOM             do { _sharpmem_vcom = _sharpmem_vcom ? 0x00 : SHARPMEM_BIT_VCOM; } while(0);

byte sharpmem_buffer[(SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8];

/* ************* */
/* CONSTRUCTORS  */
/* ************* */
Adafruit_SharpMem::Adafruit_SharpMem(uint8_t clk, uint8_t mosi, uint8_t ss) :
Adafruit_GFX(SHARPMEM_LCDWIDTH, SHARPMEM_LCDHEIGHT), _clk(clk), _mosi(mosi), _ss(ss), _sharpmem_vcom(SHARPMEM_BIT_VCOM) {
}

void Adafruit_SharpMem::begin() {
  setRotation(2);

  digitalWrite(_ss, HIGH);
  pinMode(_ss, OUTPUT);

#ifdef __AVR__
  // Set pin state before direction to make sure they start this way (no glitching)
  digitalWrite(_clk, LOW);
  digitalWrite(_mosi, HIGH);

  pinMode(_clk, OUTPUT);
  pinMode(_mosi, OUTPUT);

  clkport     = portOutputRegister(digitalPinToPort(_clk));
  clkpinmask  = digitalPinToBitMask(_clk);
  dataport    = portOutputRegister(digitalPinToPort(_mosi));
  datapinmask = digitalPinToBitMask(_mosi);
#endif
#ifndef __AVR__
  SPI.begin();
#endif
}

/* *************** */
/* PRIVATE METHODS */
/* *************** */


/**************************************************************************/
/*!
    @brief  Sends a single byte in pseudo-SPI.
*/
/**************************************************************************/
void Adafruit_SharpMem::sendbyte(uint8_t data)
{
  uint8_t i = 0;

#ifdef __AVR__
  // LCD expects LSB first
  for (i=0; i<8; i++)
  {
    // Make sure clock starts low
    *clkport &= ~clkpinmask;
    if (data & 0x80)
      *dataport |=  datapinmask;
    else
      *dataport &= ~datapinmask;

    // Clock is active high
    *clkport |=  clkpinmask;
    data <<= 1;
  }
  // Make sure clock ends low
  *clkport &= ~clkpinmask;
#else
  SPI.setBitOrder(MSBFIRST);
  SPI.transfer(data);
#endif
}

void Adafruit_SharpMem::sendbyteLSB(uint8_t data)
{
  uint8_t i = 0;

#ifdef __AVR__
  // LCD expects LSB first
  for (i=0; i<8; i++)
  {
    // Make sure clock starts low
    *clkport &= ~clkpinmask;
    if (data & 0x01)
      *dataport |=  datapinmask;
    else
      *dataport &= ~datapinmask;
    // Clock is active high
    *clkport |=  clkpinmask;
    data >>= 1;
  }
  // Make sure clock ends low
  *clkport &= ~clkpinmask;
#else
  SPI.setBitOrder(LSBFIRST);
  SPI.transfer(data);
#endif
}

/* ************** */
/* PUBLIC METHODS */
/* ************** */

// 1<<n is a costly operation on AVR -- table usu. smaller & faster
static const uint8_t PROGMEM
  set[] = {  1,  2,  4,  8,  16,  32,  64,  128 },
  clr[] = { ~1, ~2, ~4, ~8, ~16, ~32, ~64, ~128 };

/**************************************************************************/
/*!
    @brief Draws a single pixel in image buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)
*/
/**************************************************************************/
void Adafruit_SharpMem::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

  switch(rotation) {
   case 1:
    _swap_int16_t(x, y);
    x = WIDTH  - 1 - x;
    break;
   case 2:
    x = WIDTH  - 1 - x;
    y = HEIGHT - 1 - y;
    break;
   case 3:
    _swap_int16_t(x, y);
    y = HEIGHT - 1 - y;
    break;
  }

  if(color) {
    sharpmem_buffer[(y*SHARPMEM_LCDWIDTH + x) / 8] |=
      pgm_read_byte(&set[x & 7]);
  } else {
    sharpmem_buffer[(y*SHARPMEM_LCDWIDTH + x) / 8] &=
      pgm_read_byte(&clr[x & 7]);
  }
}

/**************************************************************************/
/*!
    @brief Gets the value (1 or 0) of the specified pixel from the buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)

    @return     1 if the pixel is enabled, 0 if disabled
*/
/**************************************************************************/
uint8_t Adafruit_SharpMem::getPixel(uint16_t x, uint16_t y)
{
  if((x >= _width) || (y >= _height)) return 0; // <0 test not needed, unsigned

  switch(rotation) {
   case 1:
    _swap_uint16_t(x, y);
    x = WIDTH  - 1 - x;
    break;
   case 2:
    x = WIDTH  - 1 - x;
    y = HEIGHT - 1 - y;
    break;
   case 3:
    _swap_uint16_t(x, y);
    y = HEIGHT - 1 - y;
    break;
  }

  return sharpmem_buffer[(y*SHARPMEM_LCDWIDTH + x) / 8] &
    pgm_read_byte(&set[x & 7]) ? 1 : 0;
}

/**************************************************************************/
/*!
    @brief Clears the screen
*/
/**************************************************************************/
void Adafruit_SharpMem::clearDisplay()
{
  memset(sharpmem_buffer, 0xff, (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8);
  // Send the clear screen command rather than doing a HW refresh (quicker)
  digitalWrite(_ss, HIGH);
  sendbyte(_sharpmem_vcom | SHARPMEM_BIT_CLEAR);
  sendbyteLSB(0x00);
  TOGGLE_VCOM;
  digitalWrite(_ss, LOW);
}

/**************************************************************************/
/*!
    @brief Renders the contents of the pixel buffer on the LCD
*/
/**************************************************************************/
void Adafruit_SharpMem::refresh(void)
{
  uint16_t i, totalbytes, currentline, oldline;
  totalbytes = (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8;

  // Send the write command
  digitalWrite(_ss, HIGH);
  sendbyte(SHARPMEM_BIT_WRITECMD | _sharpmem_vcom);
  TOGGLE_VCOM;

  // Send the address for line 1
  oldline = currentline = 1;
  sendbyteLSB(currentline);

  // Send image buffer
  for (i=0; i<totalbytes; i++)
  {
    sendbyteLSB(sharpmem_buffer[i]);
    currentline = ((i+1)/(SHARPMEM_LCDWIDTH/8)) + 1;
    if(currentline != oldline)
    {
      // Send end of line and address bytes
      sendbyteLSB(0x00);
      if (currentline <= SHARPMEM_LCDHEIGHT)
      {
        sendbyteLSB(currentline);
      }
      oldline = currentline;
    }
  }

  // Send another trailing 8 bits for the last line
  sendbyteLSB(0x00);
  digitalWrite(_ss, LOW);
}

/**************************************************************************/
/*!
    @brief Clears the display buffer
*/
/**************************************************************************/
void Adafruit_SharpMem::clearDisplayBuffer()
{
  memset(sharpmem_buffer, 0xff, (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8);
}

void Adafruit_SharpMem::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  boolean bSwap = false;
  switch(rotation) {
    case 0:
      // 0 degree rotation, do nothing
      break;
    case 1:
      // 90 degree rotation, swap x & y for rotation, then invert x
      bSwap = true;
      _swap_int16_t(x, y);
      x = WIDTH - x - 1;
      break;
    case 2:
      // 180 degree rotation, invert x and y - then shift y around for height.
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      x -= (w-1);
      break;
    case 3:
      // 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to become h)
      bSwap = true;
      _swap_int16_t(x, y);
      y = HEIGHT - y - 1;
      y -= (w-1);
      break;
  }

  if(bSwap) {
    drawFastVLineInternal(x, y, w, color);
  } else {
    drawFastHLineInternal(x, y, w, color);
  }
}

void Adafruit_SharpMem::drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) {
  // Do bounds/limit checks
  if(y < 0 || y >= HEIGHT) { return; }

  // make sure we don't try to draw below 0
  if(x < 0) {
    w += x;
    x = 0;
  }

  // make sure we don't go off the edge of the display
  if( (x + w) > WIDTH) {
    w = (WIDTH - x);
  }

  // if our width is now negative, punt
  if(w <= 0) { return; }

  // set up the pointer for  movement through the buffer
  register uint8_t *pBuf = sharpmem_buffer;
  // adjust the buffer pointer for the current row
  pBuf += ((y/8) * SHARPMEM_LCDWIDTH);
  // and offset x columns in
  pBuf += x;

  register uint8_t mask = 1 << (y&7);

  switch (color)
  {
  case WHITE:         while(w--) { *pBuf++ |= mask; }; break;
    case BLACK: mask = ~mask;   while(w--) { *pBuf++ &= mask; }; break;
  case INVERSE:         while(w--) { *pBuf++ ^= mask; }; break;
  }
}

void Adafruit_SharpMem::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  bool bSwap = false;
  switch(rotation) {
    case 0:
      break;
    case 1:
      // 90 degree rotation, swap x & y for rotation, then invert x and adjust x for h (now to become w)
      bSwap = true;
      _swap_int16_t(x, y);
      x = WIDTH - x - 1;
      x -= (h-1);
      break;
    case 2:
      // 180 degree rotation, invert x and y - then shift y around for height.
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      y -= (h-1);
      break;
    case 3:
      // 270 degree rotation, swap x & y for rotation, then invert y
      bSwap = true;
      _swap_int16_t(x, y);
      y = HEIGHT - y - 1;
      break;
  }

  if(bSwap) {
    drawFastHLineInternal(x, y, h, color);
  } else {
    drawFastVLineInternal(x, y, h, color);
  }
}


void Adafruit_SharpMem::drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) {

  // do nothing if we're off the left or right side of the screen
  if(x < 0 || x >= WIDTH) { return; }

  // make sure we don't try to draw below 0
  if(__y < 0) {
    // __y is negative, this will subtract enough from __h to account for __y being 0
    __h += __y;
    __y = 0;

  }

  // make sure we don't go past the height of the display
  if( (__y + __h) > HEIGHT) {
    __h = (HEIGHT - __y);
  }

  // if our height is now negative, punt
  if(__h <= 0) {
    return;
  }

  // this display doesn't need ints for coordinates, use local byte registers for faster juggling
  register uint8_t y = __y;
  register uint8_t h = __h;


  // set up the pointer for fast movement through the buffer
  register uint8_t *pBuf = sharpmem_buffer;
  // adjust the buffer pointer for the current row
  pBuf += ((y/8) * SHARPMEM_LCDWIDTH);
  // and offset x columns in
  pBuf += x;

  // do the first partial byte, if necessary - this requires some masking
  register uint8_t mod = (y&7);
  if(mod) {
    // mask off the high n bits we want to set
    mod = 8-mod;

    // note - lookup table results in a nearly 10% performance improvement in fill* functions
    // register uint8_t mask = ~(0xFF >> (mod));
    static uint8_t premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
    register uint8_t mask = premask[mod];

    // adjust the mask if we're not going to reach the end of this byte
    if( h < mod) {
      mask &= (0XFF >> (mod-h));
    }

  switch (color)
    {
    case WHITE:   *pBuf |=  mask;  break;
    case BLACK:   *pBuf &= ~mask;  break;
    case INVERSE: *pBuf ^=  mask;  break;
    }

    // fast exit if we're done here!
    if(h<mod) { return; }

    h -= mod;

    pBuf += SHARPMEM_LCDWIDTH;
  }


  // write solid bytes while we can - effectively doing 8 rows at a time
  if(h >= 8) {
    if (color == INVERSE)  {          // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
      do  {
      *pBuf=~(*pBuf);

        // adjust the buffer forward 8 rows worth of data
        pBuf += SHARPMEM_LCDWIDTH;

        // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
        h -= 8;
      } while(h >= 8);
      }
    else {
      // store a local value to work with
      register uint8_t val = (color == WHITE) ? 255 : 0;

      do  {
        // write our value in
      *pBuf = val;

        // adjust the buffer forward 8 rows worth of data
        pBuf += SHARPMEM_LCDWIDTH;

        // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
        h -= 8;
      } while(h >= 8);
      }
    }

  // now do the final partial byte, if necessary
  if(h) {
    mod = h & 7;
    // this time we want to mask the low bits of the byte, vs the high bits we did above
    // register uint8_t mask = (1 << mod) - 1;
    // note - lookup table results in a nearly 10% performance improvement in fill* functions
    static uint8_t postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
    register uint8_t mask = postmask[mod];
    switch (color)
    {
      case WHITE:   *pBuf |=  mask;  break;
      case BLACK:   *pBuf &= ~mask;  break;
      case INVERSE: *pBuf ^=  mask;  break;
    }
  }
}
