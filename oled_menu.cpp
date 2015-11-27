/*********************************************************************
This is a library for OLEDS

Written by Mark Winney.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined ESP8266 //Added for compatibility with ESP8266 BOARD
 #include <pgmspace.h>
#endif
#ifndef __SAM3X8E__
 #ifndef ESP8266
  #include <util/delay.h>
 #endif
#endif
#include <stdlib.h>

//#include "Adafruit_GFX.h"
#include "ESP8266_SSD1322.h"
#include "oled_menu.h"

#define NOINVERT	false

extern const uint8_t selectbar_top[];
extern const uint8_t selectbar_bottom[];
extern const uint8_t menu_default[];
extern const uint8_t selectbar_topWidthPixels;


OLEDMenu::OLEDMenu(ESP8266_SSD1322& display) : m_display(display)
{
}

bool OLEDMenu::downOption(void)
{
//Serial.println("MenuManager::downOption(void)");
	menus[menu_selected]->option_selected++;
	uint8_t sel = menus[menu_selected]->option_selected;

	if(sel >= menus[menu_selected]->num_options)
	{
		menus[menu_selected]->option_selected = 0;
	}
	return true;
}

bool OLEDMenu::upOption()
{
//Serial.println("MenuManager::upOption(void)");
	menus[menu_selected]->option_selected--;
	uint8_t sel = menus[menu_selected]->option_selected;

	if(sel >= menus[menu_selected]->num_options)
	{
		menus[menu_selected]->option_selected = menus[menu_selected]->num_options - 1;
	}
	return true;
}

bool OLEDMenu::selectOption(void)
{
	// Move to the next menu, assuming the option selected is a menu

//Serial.println("MenuManager::selectOption(void)");
	int8_t optSel = menus[menu_selected]->option_selected;
	pFunc funct = menus[menu_selected]->options[optSel]->func;
	bool subMenu = (funct == NULL);
//Serial.print("subMenu=");
//Serial.println(subMenu);

	if (subMenu == true)
	{
		// Get the index to the sub menu
		int8_t menuIndex = menus[menu_selected]->options[optSel]->menu_index;

		// See if this is the exit option
		if (menuIndex == -99)
		{
			menu_selected = menus[menu_selected]->prev_menu;
			// Reset the option back to zero
			//			mMenus[mMenuSelected]->mOptionSelected = 0;
		}
		else
		{
			// Store where you came from into the new menu so can get back when exit menu
			uint8_t tempMenu = menu_selected;

			// Change to the new menu
			menu_selected = menuIndex;

			// Store where you came from into the new menu so can get back when exit menu
			menus[menu_selected]->prev_menu = tempMenu;

			// Reset the option back to zero
			menus[menu_selected]->option_selected = 0;

		}
	}
	else
	{
		// Must call the function setup
		funct();
	}
	return true;
}



void OLEDMenu::initMenu(uint8_t num)
{
	num_menus = num;
	menus = new s_menu*[num];  // Allocate space for the menus.  Array of pointers to menus
	menu_selected = 0;
}

// Create a menu by allocating space
void OLEDMenu::createMenu(int8_t index, int8_t num_options, const char *name)
{
	menus[index] = new s_menu; // allocate space for the menu
	menus[index]->name = name; // Store pointer to option name
	menus[index]->options = new s_option*[num_options]; // Allocate array of pointers to options
	menus[index]->num_options = num_options;
	menus[index]->option_selected = 0;
}

void OLEDMenu::createOption(int8_t menu_index, int8_t opt_index, const char *name, const uint8_t *icon, pFunc actionFunc)
{
	menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
	menus[menu_index]->options[opt_index]->func = actionFunc;
	menus[menu_index]->options[opt_index]->icon = icon;
	menus[menu_index]->options[opt_index]->name = name;
	menus[menu_index]->options[opt_index]->menu_index = -1;
}

void OLEDMenu::createOption(int8_t menu_index, int8_t opt_index, const char *name, const uint8_t *icon, uint8_t prev_menu_index)
{
	menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
	menus[menu_index]->options[opt_index]->func = NULL;
	menus[menu_index]->options[opt_index]->icon = icon;
	menus[menu_index]->options[opt_index]->name = name;
	menus[menu_index]->options[opt_index]->menu_index = prev_menu_index;
}

bool OLEDMenu::updateMenu()
{
	// Get the first menu to start with...change later to get current
	bool bAnimating = menu_drawIcon();

	return bAnimating;
}

bool OLEDMenu::menu_drawIcon()
{
	const int16_t displayWidth = m_display.width();
	const int16_t displayHeight = m_display.height();

	bool bAnimating = true;
//	static int animX = 64;

//	int x = 64;
	int x = displayWidth / 2;
	x -= 48 * menus[menu_selected]->option_selected;

	{
		int8_t speed;
		if(x > animX)
		{
			speed = ((x - animX) / 4) + 1;
			if(speed > 16)
			speed = 16;
			animX += speed;
			if(x <= animX)
			animX = x;
		}
		else if(x < animX)
		{
			speed = ((animX - x) / 4) + 1;
			if(speed > 16)
			speed = 16;
			animX -= speed;
			if(x >= animX)
			animX = x;
		}
		else
		{
			bAnimating = false;
		}
	}

	x = animX - 16;

	// Copy the data from PROGMEM...max 20 chars
	char tmpStr[20] = {0};
	strcpy_P(tmpStr, menus[menu_selected]->name);
	m_display.drawCentreString(tmpStr, displayWidth / 2, 0, textSize);

	// Create image struct
	// FIX: struct uses heap, should use stack
	uint8_t fix = selectbar_topWidthPixels;
	s_image img = newImage((displayWidth / 2) - (selectbar_topWidthPixels / 2),
			14, selectbar_top, fix, 8, WHITE, NOINVERT);

	// Draw ...
	ultraFastDrawBitmap(&img);

	// Draw ...
	img.y = 42;
	img.bitmap = selectbar_bottom;
	ultraFastDrawBitmap(&img);

	img.y = 16;
	img.width = 32;
	img.height = 32;

	for(byte i = 0;i < menus[menu_selected]->num_options; i++)
	{
		if(x < displayWidth && x > -32)
		{
			img.x = x;
			img.bitmap = menus[menu_selected]->options[i]->icon != NULL ? menus[menu_selected]->options[i]->icon : menu_default;
			ultraFastDrawBitmap(&img);
		}
		x += 48;
	}

	uint8_t sel_opt = menus[menu_selected]->option_selected;
	strcpy_P(tmpStr, menus[menu_selected]->options[sel_opt]->name);
	m_display.drawString(tmpStr, 0, displayHeight - 8 , textSize);

	return bAnimating;
}

void OLEDMenu::ultraFastDrawBitmap(s_image* image)
{
	m_display.fastDrawBitmap(image->x, image->y, image->bitmap, image->width, image->height, image->foreColour);
}

void OLEDMenu::resetMenu()
{
	// reset all the options back to default 0
	menu_selected = 0;

	for (int menuLoop = 0; menuLoop < num_menus; menuLoop++)
	{
		menus[menuLoop]->option_selected = 0;
	}

	// Reset Animation
	animX = SSD1322_LCDWIDTH / 2;
}

void OLEDMenu::setTextSize(uint8_t size)
{
	textSize = size;
}

