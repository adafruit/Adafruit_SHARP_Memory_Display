// Sharp Memory Display library by adafruit
// MIT license

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
 
class SharpMem{
 public:
  SharpMem(uint8_t clk, uint8_t mosi, uint8_t ss, uint8_t extin, uint8_t dispen);
  void begin(void);
  void Enable(bool enable);
  void DrawPixel(uint16_t x, uint16_t y);
  void ClearPixel(uint16_t x, uint16_t y);
  uint8_t GetPixel(uint16_t x, uint16_t y);
  void ClearScreen();
  void Refresh(void);

 private:
  uint8_t _ss, _clk, _mosi, _extin, _dispen;
  uint8_t _sharpmem_vcom;
  
  uint8_t swap(uint8_t data);
  void sendbyte(uint8_t data);
};
