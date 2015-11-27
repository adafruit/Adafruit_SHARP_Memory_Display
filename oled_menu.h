/*********************************************************************
This is a library for OLEDS

Written by Mark Winney.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/
#ifndef _OLED_MENU_H
#define _OLED_MENU_H

#if ARDUINO >= 100
 #include "Arduino.h"
 #define WIRE_WRITE Wire.write
#else
 #include "WProgram.h"
  #define WIRE_WRITE Wire.send
#endif

#ifdef __SAM3X8E__
 typedef volatile RwReg PortReg;
 typedef uint32_t PortMask;
#else
  typedef volatile uint8_t PortReg;
  typedef uint8_t PortMask;
#endif

#define newImage(x, y, bitmap, width, height, foreColour, invert) \
(s_image){ \
	x, \
	y, \
	bitmap, \
	width, \
	height, \
	foreColour, \
	invert \
}

typedef struct
{
	uint8_t x;
	uint8_t y;
	const uint8_t* bitmap;
	uint8_t width;
	uint8_t height;
	uint8_t foreColour;
	bool invert;
}s_image;


typedef void (*pFunc)(void);

typedef struct
{
	int8_t num_items;
	const char *name;
	const uint8_t *icon;
	int8_t menu_index;
	pFunc func;
}s_option;

typedef struct
{
	int8_t num_options;
	const char *name;
	s_option **options; // Array of pointer to options
	int8_t option_selected;
	int8_t prev_menu;
}s_menu;

class OLEDMenu
{
  public:
	OLEDMenu(ESP8266_SSD1322& display);
	void initMenu(uint8_t num);
	void createMenu(int8_t index, int8_t num_options, const char *name);
	void createOption(int8_t menu_index, int8_t opt_index, const char *name, const uint8_t *icon, pFunc actionFunc);
	void createOption(int8_t menu_index, int8_t opt_index, const char *name, const uint8_t *icon, uint8_t prev_menu_index);
	bool updateMenu();
	bool upOption(void);
	bool downOption(void);
	bool selectOption(void);
	void resetMenu(void);
	bool menu_drawIcon();
	void setTextSize(uint8_t size);

  private:
	void ultraFastDrawBitmap(s_image* image);

	int8_t num_menus;
	s_menu **menus; //Array of pointers to menus
	uint8_t menu_selected;
	int animX;  // menu animation X pos
	ESP8266_SSD1322& m_display;
	uint8_t textSize;
};


#endif
