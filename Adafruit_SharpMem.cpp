#include "SharpMem.h"

/**************************************************************************
    Sharp Memory Display Connector
    -----------------------------------------------------------------------
    Pin   Function        Notes
    ===   ==============  ===============================
      1   VIN             1.0-5.0V (Boost Supply)
      2   VDD             5.0V (Boost output, or 5V supply)
      3   GND
      4   SCLK            Serial Clock
      5   MOSI            Serial Data Input
      6   CS              Serial Chip Select
      7   EXTCOMIN        External COM Inversion Signal
      8   DISP            Display On(High)/Off(Low)
      9   EXTMODE         COM Inversion Select (Low = SW clock/serial)

 **************************************************************************/

// LCD Dimensions
#define SHARPMEM_LCDWIDTH       (96)
#define SHARPMEM_LCDHEIGHT      (96) 
#define SHARPMEM_BIT_WRITECMD   (0x80)
#define SHARPMEM_BIT_VCOM       (0x40)
#define SHARPMEM_BIT_CLEAR      (0x20)
#define TOGGLE_VCOM             do { _sharpmem_vcom = _sharpmem_vcom ? 0x00 : SHARPMEM_BIT_VCOM; } while(0);

byte sharpmem_buffer[(SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8];

/* ************* */
/* CONSTRUCTORS  */
/* ************* */
SharpMem::SharpMem(uint8_t clk, uint8_t mosi, uint8_t ss, uint8_t extin, uint8_t dispen) 
{
  _clk = clk;
  _mosi = mosi;
  _ss = ss;
  _extin = extin;
  _dispen = dispen;

  // Set pin state before direction to make sure they start this way (no glitching)
  digitalWrite(_ss, HIGH);  
  digitalWrite(_clk, LOW);  
  digitalWrite(_mosi, HIGH);  
  digitalWrite(_extin, LOW);  
  digitalWrite(_dispen, LOW);  
  
  pinMode(_ss, OUTPUT);
  pinMode(_clk, OUTPUT);
  pinMode(_mosi, OUTPUT);
  pinMode(_extin, OUTPUT);
  pinMode(_dispen, OUTPUT);
  
  // Set the vcom bit to a defined state
  _sharpmem_vcom = SHARPMEM_BIT_VCOM;
}

void SharpMem::begin() {
}

/* *************** */
/* PRIVATE METHODS */
/* *************** */
 
/**************************************************************************/
/*!
    @brief  Swaps the bit order from MSB to LSB, since the LCD expects LSB,
            but the M3 shifts bits out MSB.
*/
/**************************************************************************/
uint8_t SharpMem::swap(uint8_t data)
{
  uint8_t out = 0;
  if (data)
  {
    if(data & 0x01) out |= 0x80;
    if(data & 0x02) out |= 0x40;
    if(data & 0x04) out |= 0x20;
    if(data & 0x08) out |= 0x10;
    if(data & 0x10) out |= 0x08;
    if(data & 0x20) out |= 0x04;
    if(data & 0x40) out |= 0x02;
    if(data & 0x80) out |= 0x01;
  }

  return out;
}
 
/**************************************************************************/
/*!
    @brief  Sends a single byte in pseudo-SPI.
*/
/**************************************************************************/
void SharpMem::sendbyte(uint8_t data) 
{
  uint8_t i = 0;

  // LCD expects LSB first
  for (i=0; i<8; i++) 
  { 
    // Make sure clock starts low
    digitalWrite(_clk, LOW);
    if (data & 0x80) 
    { 
      digitalWrite(_mosi, HIGH);
    } 
    else 
    { 
      digitalWrite(_mosi, LOW);
    } 
    // Clock is active high
    digitalWrite(_clk, HIGH);
    data <<= 1; 
  }
  // Make sure clock ends low
  digitalWrite(_clk, LOW);
}

/* ************** */
/* PUBLIC METHODS */
/* ************** */

/**************************************************************************/
/*! 
    @brief Turns the display on or off (memory is retained even when the
           display is off)

    @param[in]  enabled
                Whether the display should be on (TRUE/1) or off (FALSE/0)
*/
/**************************************************************************/
void SharpMem::Enable(bool enable)
{
  if (enable)
  {
    digitalWrite(_dispen, HIGH);  
  }
  else
  {
    digitalWrite(_dispen, LOW);  
  }
}

/**************************************************************************/
/*! 
    @brief Draws a single pixel in image buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)
*/
/**************************************************************************/
void SharpMem::DrawPixel(uint16_t x, uint16_t y) 
{
  if ((x >= SHARPMEM_LCDWIDTH) || (y >= SHARPMEM_LCDHEIGHT))
    return;

  sharpmem_buffer[(y*SHARPMEM_LCDWIDTH + x) /8] |= (1 << x % 8);
}

/**************************************************************************/
/*! 
    @brief Clears a single pixel in image buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)
*/
/**************************************************************************/
void SharpMem::ClearPixel(uint16_t x, uint16_t y) 
{
  if ((x >= SHARPMEM_LCDWIDTH) || (y >= SHARPMEM_LCDHEIGHT))
    return;

  sharpmem_buffer[(y*SHARPMEM_LCDWIDTH + x) /8] &= ~(1 << x % 8);
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
uint8_t SharpMem::GetPixel(uint16_t x, uint16_t y)
{
  if ((x >=SHARPMEM_LCDWIDTH) || (y >=SHARPMEM_LCDHEIGHT)) return 0;
  return sharpmem_buffer[(y*SHARPMEM_LCDWIDTH + x) /8] & (1 << x % 8) ? 1 : 0;
}

/**************************************************************************/
/*! 
    @brief Clears the screen
*/
/**************************************************************************/
void SharpMem::ClearScreen() 
{
  memset(sharpmem_buffer, 0x00, (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8);
  // Send the clear screen command rather than doing a HW refresh (quicker)
  digitalWrite(_ss, HIGH);
  sendbyte(_sharpmem_vcom | SHARPMEM_BIT_CLEAR);
  sendbyte(0x00);
  TOGGLE_VCOM;
  digitalWrite(_ss, LOW);
}

/**************************************************************************/
/*! 
    @brief Renders the contents of the pixel buffer on the LCD
*/
/**************************************************************************/
void SharpMem::Refresh(void) 
{
  uint16_t i, totalbytes, currentline, oldline;  
  totalbytes = (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8;

  // Send the write command
  digitalWrite(_ss, HIGH);
  sendbyte(SHARPMEM_BIT_WRITECMD | _sharpmem_vcom);
  TOGGLE_VCOM;

  // Send the address for line 1
  oldline = currentline = 1;
  sendbyte(swap(currentline));

  // Send image buffer
  for (i=0; i<totalbytes; i++)
  {
    sendbyte(swap(sharpmem_buffer[i]));
    currentline = ((i+1)/(SHARPMEM_LCDWIDTH/8)) + 1;
    if(currentline != oldline)
    {
      // Send end of line and address bytes
      sendbyte(0x00);
      if (currentline <= SHARPMEM_LCDHEIGHT)
      {
        sendbyte(swap(currentline));
      }
      oldline = currentline;
    }
  }

  // Send another trailing 8 bits for the last line
  sendbyte(0x00);
  digitalWrite(_ss, LOW);
}
