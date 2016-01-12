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

OLEDMenu::OLEDMenu (ESP8266_SSD1322& display) :
    m_display (display)
{
}

bool
OLEDMenu::downOption (void)
{
Serial.print("downOption:");
Serial.println(menu_selected);

//Serial.println("MenuManager::downOption(void)");
  menus[menu_selected]->option_selected++;
  uint8_t sel = menus[menu_selected]->option_selected;

  if (sel >= menus[menu_selected]->num_options)
  {
    menus[menu_selected]->option_selected = 0;
  }
  return true;
}

bool
OLEDMenu::upOption ()
{
Serial.print("upOption:");
Serial.println(menu_selected);

  //Serial.println("MenuManager::upOption(void)");
  menus[menu_selected]->option_selected--;
  uint8_t sel = menus[menu_selected]->option_selected;

  if (sel >= menus[menu_selected]->num_options)
  {
    menus[menu_selected]->option_selected = menus[menu_selected]->num_options
	- 1;
  }
  return true;
}

bool
OLEDMenu::selectOption (void)
{
  // Move to the next menu, assuming the option selected is a menu

//Serial.println("MenuManager::selectOption(void)");
  int8_t optSel = menus[menu_selected]->option_selected;
  pFunc funct = menus[menu_selected]->options[optSel]->func;
  bool subMenu = (funct == NULL);
Serial.println("-------------------------");
Serial.print("menu_selected=");
Serial.println(menu_selected);
Serial.print("optSel=");
Serial.println(optSel);
Serial.print("subMenu=");
Serial.println(subMenu);

  if (subMenu == true)
  {
    // Get the index to the sub menu
    int8_t menuIndex = menus[menu_selected]->options[optSel]->menu_index;

Serial.print("menuIndex=");
Serial.println(menuIndex);
    // See if this is the exit option
    if (menuIndex == MENU_EXIT)
    {
Serial.print("menu_selected=");
Serial.println(menu_selected);
      menu_selected = menus[menu_selected]->prev_menu;
Serial.print("menu_selected=");
Serial.println(menu_selected);
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
    funct ();
  }
  return true;
}

void
OLEDMenu::initMenu (uint8_t num)
{
  num_menus = num;
  menus = new s_menu*[num]; // Allocate space for the menus.  Array of pointers to menus
  menu_selected = 0;
}

// Create a menu by allocating space
void
OLEDMenu::createMenu (int8_t index, int8_t num_options, const char *name)
{
  menus[index] = new s_menu; // allocate space for the menu
  menus[index]->name = name; // Store pointer to option name
  menus[index]->options = new s_option*[num_options]; // Allocate array of pointers to options
  menus[index]->num_options = num_options;
  menus[index]->option_selected = 0;
}

void
OLEDMenu::createOption (int8_t menu_index, int8_t opt_index, const char *name,
			const uint8_t *icon, pFunc actionFunc)
{
  menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
  menus[menu_index]->options[opt_index]->func = actionFunc;
  menus[menu_index]->options[opt_index]->icon = icon;
  menus[menu_index]->options[opt_index]->name = name;
  menus[menu_index]->options[opt_index]->menu_index = -1;
}

void
OLEDMenu::createOption (int8_t menu_index, int8_t opt_index, const char *name,
			const uint8_t *icon, uint8_t prev_menu_index)
{
  menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
  menus[menu_index]->options[opt_index]->func = NULL;
  menus[menu_index]->options[opt_index]->icon = icon;
  menus[menu_index]->options[opt_index]->name = name;
  menus[menu_index]->options[opt_index]->menu_index = prev_menu_index;
}

void
OLEDMenu::createOption (int8_t menu_index, int8_t opt_index, pFunc actionFunc,
			uint8_t prev_menu_index)
{
  menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
  menus[menu_index]->options[opt_index]->func = actionFunc;
//	menus[menu_index]->options[opt_index]->icon = icon;
//	menus[menu_index]->options[opt_index]->name = name;
  menus[menu_index]->options[opt_index]->menu_index = prev_menu_index;
}

void
OLEDMenu::createOption (int8_t menu_index, int8_t opt_index, const char *name,
			uint8_t prev_menu_index)
{
  menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
//	menus[menu_index]->options[opt_index]->func = actionFunc;
//	menus[menu_index]->options[opt_index]->icon = icon;
  menus[menu_index]->options[opt_index]->name = name;
  menus[menu_index]->options[opt_index]->menu_index = prev_menu_index;
}

bool
OLEDMenu::updateMenu ()
{
  bool bAnimating = false;

  // Get the index to the sub menu
//	int8_t optSel = menus[menu_selected]->option_selected;
//	int8_t menuIndex = menus[menu_selected]->options[optSel]->menu_index;
//
//	// See if this option was to display a paged display
//	if (menuIndex == MENU_PAGED_NEXT_PREV)
//	{
//Serial.println("MENU_PAGED_NEXT_PREV");
//	}
//	else
  {
    // Display as regular icon
    // Get the first menu to start with...change later to get current
    bAnimating = menu_drawIcon ();
  }

  return bAnimating;
}

bool
OLEDMenu::menu_drawIcon ()
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
    if (x > animX)
    {
      speed = ((x - animX) / 4) + 1;
      if (speed > 16)
	speed = 16;
      animX += speed;
      if (x <= animX)
	animX = x;
    }
    else if (x < animX)
    {
      speed = ((animX - x) / 4) + 1;
      if (speed > 16)
	speed = 16;
      animX -= speed;
      if (x >= animX)
	animX = x;
    }
    else
    {
      bAnimating = false;
    }
  }

  x = animX - 16;

  // Copy the data from PROGMEM...max 20 chars
  char tmpStr[20] = { 0 };
  strcpy_P (tmpStr, menus[menu_selected]->name);
  m_display.drawCentreString (tmpStr, displayWidth / 2, 0, textSize);

  // Create image struct
  // FIX: struct uses heap, should use stack
  uint8_t fix = selectbar_topWidthPixels;
  s_image img = newImage((displayWidth / 2) - (selectbar_topWidthPixels / 2),
			 14, selectbar_top, fix, 8, WHITE, NOINVERT);

  // Draw ...
  ultraFastDrawBitmap (&img);

  // Draw ...
  img.y = 42;
  img.bitmap = selectbar_bottom;
  ultraFastDrawBitmap(&img);

  img.y = 16;
  img.width = 32;
  img.height = 32;

  // Display each menu option
  for (byte i = 0; i < menus[menu_selected]->num_options; i++)
  {
    if (x < displayWidth && x > -32)
    {
      img.x = x;
      img.bitmap =
	  menus[menu_selected]->options[i]->icon != NULL ?
	      menus[menu_selected]->options[i]->icon : menu_default;
      ultraFastDrawBitmap(&img);
    }
    x += 48;
  }

  uint8_t sel_opt = menus[menu_selected]->option_selected;
  strcpy_P (tmpStr, menus[menu_selected]->options[sel_opt]->name);
  m_display.drawString(tmpStr, 0, displayHeight - 8, textSize);

  return bAnimating;
}

void
OLEDMenu::ultraFastDrawBitmap (s_image* image)
{
  m_display.fastDrawBitmap(image->x, image->y, image->bitmap, image->width,
			    image->height, image->foreColour);
}

void
OLEDMenu::resetMenu ()
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

void
OLEDMenu::setTextSize (uint8_t size)
{
  textSize = size;
}
// --------------------------------------------------------------------------------------------//

OLEDPage::OLEDPage (ESP8266_SSD1322& display, int8_t itemsPerPage) :
    m_display (display), m_currentMenuOption (NONE), m_currentPage (0), m_itemsPerPage (
	itemsPerPage)
{
}

void OLEDPage::begin()
{
  m_currentMenuOption = NONE;
  m_currentPage = 0;
}


void
OLEDPage::display (uint8_t currentPage)
{
  // Save the current page in the class
  m_currentPage = currentPage;

  if (m_currentMenuOption == NEXT_PAGE)
  {
    m_display.setTextColor (BLACK, WHITE);
  }
  else
  {
    // Display "Next Page" and "Exit" on each page
    m_display.setTextColor (WHITE);
  }

  // Only display this if more then 1 page
  m_display.setCursor (0, 0);
  m_display.print (F ("<Next Page>"));

  if (currentPage > 0)
  {
    if (m_currentMenuOption == PREV_PAGE)
    {
      m_display.setTextColor (BLACK, WHITE);
    }
    else
    {
      // Display "Next Page" and "Exit" at the bottom of each page
      m_display.setTextColor (WHITE);
    }

    // Only display this not on page 1
    m_display.setCursor (70, 0);
    m_display.print (F ("<Prev Page>"));
  }

  if (m_currentMenuOption == EXIT_PAGE)
  {
    m_display.setTextColor (BLACK, WHITE);
  }
  else
  {
    // Display "Next Page" and "Exit" at the bottom of each page
    m_display.setTextColor (WHITE);
  }

  m_display.setCursor (140, 0);
  m_display.print (F ("<Exit>"));
}

// returns true if we are going into the menu else false
bool
OLEDPage::handleUpOption (int8_t& selected)
{
Serial.print("handleUpOption:");
Serial.println(selected);
  // If we deduct 1 from this do we exit from the
  // page data and head into the menu.  We know this
  // if the selected passed is < 0

  int8_t newSel = selected - 1;

  // Check if into the menu area.
  if (newSel < 0)
  {
    // Must be into the menu at top of page now
    m_currentMenuOption++;

    // Check the current page and if we are
    // on page 1 then the Prev option is not displayed
    // so move straight to the exit option.
    if (m_currentPage == 0 && m_currentMenuOption == PREV_PAGE)
    {
      m_currentMenuOption++;
    }

    // If we are past the exit page then go back to the
    // Next Page option
    if (m_currentMenuOption > EXIT_PAGE)
    {
      m_currentMenuOption = NEXT_PAGE;
    }
//Serial.println("Into Menu");
    return true;
  }
  else
  {
    // Not in the menu, but in the list so send back a new
    // selected item.
    selected--;

//		if (selected < 0)
//		{
//		  // Roll to last page
//			selected = stationSize - 1;
//		}
  }

  return false;
}

// returns true if we are going into the menu else false
bool
OLEDPage::handleDownOption (int8_t& selected)
{
Serial.print("handleDownOption:");
Serial.println(selected);
  // Check if we are in the menu and if the down is selected
  // go back into the list and get out of the menu.
  if (m_currentMenuOption != NONE)
  {
    // If we have just exited the menu then
    // reset to top of the page
    selected = 0;
    m_currentMenuOption = NONE;
    return true;
  }
  else
  {
    // Move to the next item on the page
    selected++;

    // See if we are on the last page and
    // make sure when we move past the end
    // of the list that we go back to the start.
    bool lastPage = (m_currentPage == (m_totalItemsInList / m_itemsPerPage));
    if (lastPage)
    {
      uint8_t lastItem = m_totalItemsInList - (m_currentPage * m_itemsPerPage)
	  - 1;

      if (selected > lastItem)
      {
	selected = 0;
      }
    }
    else
    {
      // Rotate back to start of the page if
      // we reached the end.
      if (selected > (m_itemsPerPage - 1))
      {
	selected = 0;
      }
    }
  }

  return false;
}

// returns true if we are going into the menu else false
OLEDPage::page_options
OLEDPage::handleSelectOption (int8_t& currentPageNumber)
{
  Serial.println (m_currentMenuOption);

  if (m_currentMenuOption == NEXT_PAGE)
  {
    // Make sure don't go past last page
    bool lastPage = (m_currentPage == (m_totalItemsInList / m_itemsPerPage));
    if (!lastPage)
    {
      currentPageNumber++;
    }
  }
  else if (m_currentMenuOption == PREV_PAGE)
  {
    currentPageNumber--;

    // If this is the first page then no Prev Page
    // will be displayed so reset to Next Page
    if (currentPageNumber == 0)
    {
      m_currentMenuOption = NEXT_PAGE;
    }
  }
  else if (m_currentMenuOption == EXIT_PAGE)
  {
    m_currentMenuOption = NONE;
    return EXIT_PAGE;
  }

  return (page_options) m_currentMenuOption;
}

// --------------------------------------------------------------------------------------------//

OLEDKeyboard::OLEDKeyboard(ESP8266_SSD1322& display): m_display(display)
{
}

void OLEDKeyboard::display(uint8_t currentPage)
{
  uint8_t xpos = 50;
  uint8_t ypos = 32;

  m_display.setTextSize(1);
  m_display.setTextColor(WHITE);
  m_display.setCursor(xpos, ypos);

  // Display the numbers 0-9 at top
  for (uint8_t i = 0; i < 10; i++)
  {
    m_display.setCursor(xpos + 10, ypos);
    m_display.write(48 + i);
  }
}
void OLEDKeyboard::handleUpOption(int8_t& selected)
{

}
void OLEDKeyboard::handleDownOption(int8_t& selected)
{

}
void OLEDKeyboard::handleSelectOption()
{

}

