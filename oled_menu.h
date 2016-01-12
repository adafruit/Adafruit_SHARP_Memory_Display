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

#define MENU_EXIT              -99
#define OPTION_NEXT_PAGE       -1
#define OPTION_PREV_PAGE       -2
#define OPTION_EXIT_PAGE       -3

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
	int16_t x;
	int16_t y;
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

class OLEDKeyboard
{
public:
  OLEDKeyboard(ESP8266_SSD1322& display);
  void display(uint8_t currentPage);
  void handleUpOption(int8_t& selected);
  void handleDownOption(int8_t& selected);
  void handleSelectOption();

private:
  ESP8266_SSD1322& m_display;

};

class OLEDPage
{
public:
	enum options {NONE = 0, NEXT_PAGE, PREV_PAGE, EXIT_PAGE};
	typedef options page_options;

	OLEDPage(ESP8266_SSD1322& display, int8_t itemsPerPage);
	void begin();
	void setTotalItemsInList(int16_t totalItemsInList) { m_totalItemsInList = totalItemsInList; };
	void display(uint8_t currentPage);
	bool handleUpOption(int8_t& selected);
	bool handleDownOption(int8_t& selected);
	page_options handleSelectOption(int8_t& currentPageNumber);


private:
	ESP8266_SSD1322& m_display;
	int8_t m_currentMenuOption; // Maps to the enum above
	int8_t m_currentPage;
	int8_t m_itemsPerPage;
	int16_t m_totalItemsInList = 0;
};

class OLEDMenu
{
public:
	OLEDMenu(ESP8266_SSD1322& display);
	void initMenu(uint8_t num);
	void createMenu(int8_t index, int8_t num_options, const char *name);
	void createOption(int8_t menu_index, int8_t opt_index, const char *name, const uint8_t *icon, pFunc actionFunc);
	void createOption(int8_t menu_index, int8_t opt_index, const char *name, const uint8_t *icon, uint8_t prev_menu_index);
	void createOption(int8_t menu_index, int8_t opt_index, pFunc actionFunc, uint8_t prev_menu_index);
	void createOption(int8_t menu_index, int8_t opt_index, const char *name, uint8_t prev_menu_index);
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
