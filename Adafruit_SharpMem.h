// Sharp Memory Display library by adafruit
// MIT license

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
 
#include <Adafruit_GFX.h>
#include <avr/pgmspace.h>

// LCD Dimensions
#define SHARPMEM_LCDWIDTH       (128)
#define SHARPMEM_LCDHEIGHT      (96) 

class Adafruit_SharpMem : public Adafruit_GFX {
 public:
  Adafruit_SharpMem(uint8_t clk, uint8_t mosi, uint8_t ss, uint8_t extin, uint8_t dispen);
  void begin(void);
  void enable(bool enable);
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  uint8_t getPixel(uint16_t x, uint16_t y);
  void clearDisplay();
  void refresh(void);

 private:
  uint8_t _ss, _clk, _mosi, _extin, _dispen;
  volatile uint8_t *dataport, *clkport;
  uint8_t _sharpmem_vcom, datapinmask, clkpinmask;
  
  void sendbyte(uint8_t data);
  void sendbyteLSB(uint8_t data);
};
