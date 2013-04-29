/* Draws a line across the screen */

#include <SharpMem.h>

#define SCK 2
#define MOSI 3
#define SS 4
#define EXTIN 6
#define DISPEN 7

SharpMem sharpmem(SCK, MOSI, SS, EXTIN, DISPEN);

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Hello!");

  sharpmem.begin();
  sharpmem.ClearScreen();
  
  uint32_t i;
  for (i = 0; i < 96; i++)
  {
    sharpmem.DrawPixel(i, i);
  }

  // Turn the screen on  
  sharpmem.Enable(true);
}

void loop(void) 
{
  // Screen must be refreshed at least once per second
  sharpmem.Refresh();
  delay(500);
}

