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
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif
#ifndef _swap_uint16_t
#define _swap_uint16_t(a, b)                                                   \
  {                                                                            \
    uint16_t t = a;                                                            \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

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

#define TOGGLE_VCOM                                                            \
  do {                                                                         \
    _sharpmem_vcom = _sharpmem_vcom ? 0x00 : SHARPMEM_BIT_VCOM;                \
  } while (0);

/**
 * @brief Construct a new Adafruit_SharpMem object with software SPI
 *
 * @param clk The clock pin
 * @param mosi The MOSI pin
 * @param cs The display chip select pin - **NOTE** this is ACTIVE HIGH!
 * @param width The display width
 * @param height The display height
 * @param freq The SPI clock frequency desired (unlikely to be that fast in soft
 * spi mode!)
 */
Adafruit_SharpMem::Adafruit_SharpMem(uint8_t clk, uint8_t mosi, uint8_t cs,
                                     uint16_t width, uint16_t height,
                                     uint32_t freq)
    : Adafruit_GFX(width, height) {
  _cs = cs;
  if (spidev) {
    delete spidev;
  }
  spidev =
      new Adafruit_SPIDevice(cs, clk, -1, mosi, freq, SPI_BITORDER_LSBFIRST);
}

/**
 * @brief Construct a new Adafruit_SharpMem object with hardware SPI
 *
 * @param theSPI Pointer to hardware SPI device you want to use
 * @param cs The display chip select pin - **NOTE** this is ACTIVE HIGH!
 * @param width The display width
 * @param height The display height
 * @param freq The SPI clock frequency desired
 */
Adafruit_SharpMem::Adafruit_SharpMem(SPIClass *theSPI, uint8_t cs,
                                     uint16_t width, uint16_t height,
                                     uint32_t freq)
    : Adafruit_GFX(width, height) {
  _cs = cs;
  if (spidev) {
    delete spidev;
  }
  spidev = new Adafruit_SPIDevice(cs, freq, SPI_BITORDER_LSBFIRST, SPI_MODE0,
                                  theSPI);
}

/**
 * @brief Start the driver object, setting up pins and configuring a buffer for
 * the screen contents
 *
 * @return boolean true: success false: failure
 */
boolean Adafruit_SharpMem::begin(void) {
  if (!spidev->begin()) {
    return false;
  }
  // this display is weird in that _cs is active HIGH not LOW like every other
  // SPI device
  digitalWrite(_cs, LOW);

  // Set the vcom bit to a defined state
  _sharpmem_vcom = SHARPMEM_BIT_VCOM;

  sharpmem_buffer = (uint8_t *)malloc((WIDTH * HEIGHT) / 8);

  if (!sharpmem_buffer)
    return false;

  setRotation(0);

  return true;
}

// 1<<n is a costly operation on AVR -- table usu. smaller & faster
static const uint8_t PROGMEM set[] = {1, 2, 4, 8, 16, 32, 64, 128},
                             clr[] = {(uint8_t)~1,  (uint8_t)~2,  (uint8_t)~4,
                                      (uint8_t)~8,  (uint8_t)~16, (uint8_t)~32,
                                      (uint8_t)~64, (uint8_t)~128};

/**************************************************************************/
/*!
    @brief Draws a single pixel in image buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)
    @param color The color to set:
    * **0**: Black
    * **1**: White
*/
/**************************************************************************/
void Adafruit_SharpMem::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
    return;

  switch (rotation) {
  case 1:
    _swap_int16_t(x, y);
    x = WIDTH - 1 - x;
    break;
  case 2:
    x = WIDTH - 1 - x;
    y = HEIGHT - 1 - y;
    break;
  case 3:
    _swap_int16_t(x, y);
    y = HEIGHT - 1 - y;
    break;
  }

  if (color) {
    sharpmem_buffer[(y * WIDTH + x) / 8] |= pgm_read_byte(&set[x & 7]);
  } else {
    sharpmem_buffer[(y * WIDTH + x) / 8] &= pgm_read_byte(&clr[x & 7]);
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
uint8_t Adafruit_SharpMem::getPixel(uint16_t x, uint16_t y) {
  if ((x >= _width) || (y >= _height))
    return 0; // <0 test not needed, unsigned

  switch (rotation) {
  case 1:
    _swap_uint16_t(x, y);
    x = WIDTH - 1 - x;
    break;
  case 2:
    x = WIDTH - 1 - x;
    y = HEIGHT - 1 - y;
    break;
  case 3:
    _swap_uint16_t(x, y);
    y = HEIGHT - 1 - y;
    break;
  }

  return sharpmem_buffer[(y * WIDTH + x) / 8] & pgm_read_byte(&set[x & 7]) ? 1
                                                                           : 0;
}

/**************************************************************************/
/*!
    @brief Clears the screen
*/
/**************************************************************************/
void Adafruit_SharpMem::clearDisplay() {
  memset(sharpmem_buffer, 0xff, (WIDTH * HEIGHT) / 8);

  spidev->beginTransaction();
  // Send the clear screen command rather than doing a HW refresh (quicker)
  digitalWrite(_cs, HIGH);

  uint8_t clear_data[2] = {(uint8_t)(_sharpmem_vcom | SHARPMEM_BIT_CLEAR),
                           0x00};
  spidev->transfer(clear_data, 2);

  TOGGLE_VCOM;
  digitalWrite(_cs, LOW);
  spidev->endTransaction();
}

/**************************************************************************/
/*!
    @brief Renders the contents of the pixel buffer on the LCD
*/
/**************************************************************************/
void Adafruit_SharpMem::refresh(void) {
  uint16_t i, currentline;

  spidev->beginTransaction();
  // Send the write command
  digitalWrite(_cs, HIGH);

  spidev->transfer(_sharpmem_vcom | SHARPMEM_BIT_WRITECMD);
  TOGGLE_VCOM;

  uint8_t bytes_per_line = WIDTH / 8;
  uint16_t totalbytes = (WIDTH * HEIGHT) / 8;

  for (i = 0; i < totalbytes; i += bytes_per_line) {
    uint8_t line[bytes_per_line + 2];

    // Send address byte
    currentline = ((i + 1) / (WIDTH / 8)) + 1;
    line[0] = currentline;
    // copy over this line
    memcpy(line + 1, sharpmem_buffer + i, bytes_per_line);
    // Send end of line
    line[bytes_per_line + 1] = 0x00;
    // send it!
    spidev->transfer(line, bytes_per_line + 2);
  }

  // Send another trailing 8 bits for the last line
  spidev->transfer(0x00);
  digitalWrite(_cs, LOW);
  spidev->endTransaction();
}

/**************************************************************************/
/*!
    @brief Clears the display buffer without outputting to the display
*/
/**************************************************************************/
void Adafruit_SharpMem::clearDisplayBuffer() {
  memset(sharpmem_buffer, 0xff, (WIDTH * HEIGHT) / 8);
}
