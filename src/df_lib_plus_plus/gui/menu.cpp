/*
This module implements a Menu system very similar to the standard Platform SDK
menus on Microsoft Windows. However, it does so portably. The design is a bit
weird:

- Menu configuration is loaded from a text file.
- When the config file is parsed, a keyboard shortcut for each menu is registered
  with the keyboard shortcut manager. ie Alt+F is registered for the File menu.
- However, most of the keyboard input is managed internally by this module. It
  seems like quite a good idea for all the global keyboard shortcuts to be
  managed in one place. Most of the keyboard input for this module isn't global.
  ie the keyboard input only works when the menus are in key-capture mode.
*/

// Own header
#include "menu.h"

// Project headers
#include "string_utils.h"
#include "df_lib_plus_plus/andy_string.h"
#include "df_lib_plus_plus/filesys_utils.h"
#include "df_lib_plus_plus/text_stream_readers.h"
#include "gui/drawing_primitives.h"
#include "gui/gui_base.h"
#include "gui/keyboard_shortcuts.h"

// Contrib headers
#include "df_input.h"
#include "df_bitmap.h"
#include "df_font.h"
#include "df_window.h"

// Standard headers
#include <stdlib.h>
#include <string.h>


#define MENU_SPACING_X			18
#define MENU_SPACING_Y			4
#define	HIGHLIGHT_BOX_BORDER	8
#define CONFIG_FILE_NAME		"data/config_menus.txt"


#define g_propFont g_gui->m_propFont


// ****************************************************************************
// Class MenuItem
// ****************************************************************************

MenuItem::MenuItem()
:	m_label(NULL),
	m_objectName(NULL),
	m_commandName(NULL),
	m_hotKeyIndex(0),
	m_shortcut(NULL),
	m_arguments(NULL)
{
}


MenuItem::~MenuItem()
{
	delete[] m_shortcut;
}


void MenuItem::Execute()
{
	if (m_objectName == NULL) return;

    MenuBar *menuBar = (MenuBar*)g_gui->GetWidgetByName(MENU_BAR_NAME);
	menuBar->ClearAllState();
	g_commandSender.SendCommandNoRV("MenuItem", m_objectName, m_commandName, m_arguments);
}


// ****************************************************************************
// Class Menu
// ****************************************************************************

Menu::Menu(char const *name, Widget *parent)
:	Widget(name, parent),
	m_highlightedItem(-1),
	m_longestShortcutLen(0)
{
	m_name = StringDuplicate(name);
}


void Menu::AddItem(MenuItem *item)
{
	if (item->m_objectName && item->m_commandName)
	{
		KeyboardShortcut const *shortcut = g_keyboardShortcutManager->LookupShortcut(item->m_objectName, item->m_commandName, item->m_arguments);

		if (shortcut)
		{
			char buf[64] = "";
			if (shortcut->m_keyQualifiers & KeyboardShortcut::ALT) strcat(buf, "Alt+");
			if (shortcut->m_keyQualifiers & KeyboardShortcut::CTRL) strcat(buf, "Ctrl+");
			if (shortcut->m_keyQualifiers & KeyboardShortcut::SHIFT) strcat(buf, "Shift+");
			strcat(buf, GetKeyName(shortcut->m_key));
			item->m_shortcut = StringDuplicate(buf);

			int len = GetTextWidth(g_propFont, buf);
			if (len > m_longestShortcutLen)
			{
				m_longestShortcutLen = len;
			}
		}
	}

	m_items.PutData(item);
}


int Menu::GetWidthInPixels()
{
	int longest = 0;
	for (int i = 0; i < m_items.Size(); ++i)
	{
		MenuItem *item = m_items[i];
		int width = GetTextWidth(g_propFont, item->m_label);
		if (width > longest) longest = width;
	}

	return longest + 30 + m_longestShortcutLen;
}


int Menu::GetHeightInPixels()
{
	MenuItem *item = m_items[m_items.Size() - 1];
	int rv = item->m_yPos;
	rv += g_propFont->charHeight;
	rv += MENU_SPACING_Y;
	rv += 5;

	return rv;
}


// Advance with menu displayed
//	If key up/down, set new highlighted item
//	If mouse over != highlighted item, set new highlighted item
//	If enter pressed, execute menu item
//	If lmb up,
//	  if execute menu item
//  If hot key pressed execute corresponding menu item
void Menu::Advance()
{
	// Tell the rest of the app that we are capturing mouse clicks
	if (g_gui->m_cursorManager.m_captured == NULL)
	{
		g_gui->m_cursorManager.m_captured = this;
	}

	// If key up/down, set new highlighted item
	if (g_input.keyDowns[KEY_DOWN] || g_input.keyDowns[KEY_UP])
	{
		int direction = 0;
		if (g_input.keyDowns[KEY_DOWN]) direction = 1;
		if (g_input.keyDowns[KEY_UP]) direction = -1;
		if (direction)
		{
			do
			{
				m_highlightedItem += direction;
				m_highlightedItem += m_items.Size();
				m_highlightedItem %= m_items.Size();
			} while (!m_items[m_highlightedItem]->m_commandName);
		}
	}

	bool const mouseMoved = (abs(g_input.mouseVelX) > 0 ||
							 abs(g_input.mouseVelY) > 0);

	// Update highlighted menu item based on mouse position
	if (mouseMoved)
	{
		if (IsMouseOver())
		{
			int itemNum = IsMouseOverItem();
			if (itemNum != -1 && itemNum != m_highlightedItem)
			{
                g_gui->m_canSleep = false;
				m_highlightedItem = itemNum;
			}
		}
		else
		{
			m_highlightedItem = -1;
		}
	}

	// If a hot key has been pressed, execute corresponding menu item
	for (int i = 0; i < g_input.numKeysTyped; i++)
	{
		MenuItem *item = FindMenuItemByHotkey(g_input.keysTyped[i]);
		if (item)
		{
			item->Execute();
			return;
		}
	}

	// If item clicked on or return pressed, execute highlighted menu item
	if (m_highlightedItem != -1 &&
		(g_input.keyDowns[KEY_ENTER] || g_input.lmbUnClicked))
	{
        g_gui->m_cursorManager.m_captured = NULL;
		MenuItem *item = m_items[m_highlightedItem];
		item->Execute();
	}
}


// highlightedItem should be -1 if no item is highlighted
void Menu::Render()
{
	// Calculate how tall and wide the menu needs to be
	int height = GetHeightInPixels();
	int width = GetWidthInPixels();
	int x = m_left - 8;
	int y = m_top;

	// Draw the menu background
	DrawRaisedBox(x,   y,   width + 2, height + 2, g_gui->m_frameColour1, g_gui->m_frameColour4);
	DrawFilledBox(x+1, y+1, width, height, g_gui->m_frameColour2);

	// Set mouse cursor if mouse is over menu area
	if (g_input.mouseX > x && g_input.mouseX < (x + width + 1))
	{
		if (g_input.mouseY > y && g_input.mouseY < (y + height + 1))
		{
			g_gui->m_cursorManager.RequestCursorType(CursorManager::CursorMain);
		}
	}

	int ySize = g_propFont->charHeight + MENU_SPACING_Y;
	y += 4;

	// Highlight the selected item
	if (m_highlightedItem >= 0 && m_highlightedItem < m_items.Size())
	{
		DrawFilledBox(x + 2, m_items[m_highlightedItem]->m_yPos + y, width - 3, ySize, g_gui->m_frameColour1);
	}

	// Draw each menu item
	for (int i = 0; i < m_items.Size(); ++i)
	{
		MenuItem *item = m_items[i];

		// Is item a separator?
		if (item->m_commandName)
		{
			// Not a separator

			// Item label text
			DrawTextSimple(g_propFont, g_gui->m_textColourFrame, g_window->bmp, x + 10, item->m_yPos + y, item->m_label);

			if (item->m_hotKeyIndex >= 0)
			{
				int offset = GetTextWidth(g_propFont, item->m_label, item->m_hotKeyIndex);
				int len = GetTextWidth(g_propFont, item->m_label + item->m_hotKeyIndex, 1);

				// Underline hot key
				DrawHLine(x + 10 + offset, item->m_yPos + y + g_propFont->charHeight, len, g_gui->m_textColourFrame);
			}

			// Shortcut
			if (item->m_shortcut)
			{
				DrawTextRight(g_propFont, g_gui->m_textColourFrame, g_window->bmp, x + width - 7, item->m_yPos + y, item->m_shortcut);
			}
		}
		else
		{
			// Item is a separator
			DrawHLine(x + 7, item->m_yPos + y + 3, width - 10, g_gui->m_frameColour4);
			DrawHLine(x + 7, item->m_yPos + y + 4, width - 10, g_gui->m_frameColour1);
		}
	}
}


bool Menu::IsMouseOverTitle()
{
	const int border = MENU_SPACING_X / 2;
	if (g_input.mouseX >= (m_left - border) &&
		g_input.mouseX < (m_right + border) &&
		g_input.mouseY < 18)
	{
		return true;
	}

	return false;
}


bool Menu::IsMouseOver()
{
	int menuTop = m_top; //m_parent->m_height;
	int width = GetWidthInPixels();
	int height = GetHeightInPixels();
	int mouseX = g_input.mouseX;
	if (mouseX >= m_left &&
		mouseX < m_left + width &&
		g_input.mouseY >= menuTop &&
		g_input.mouseY < (menuTop + height))
	{
		return true;
	}

	return false;
}


int Menu::IsMouseOverItem()
{
	int ySize = g_propFont->charHeight + MENU_SPACING_Y;
	for (int i = 0; i < m_items.Size(); ++i)
	{
		MenuItem *item = m_items[i];

		// Skip separators
		if (item->m_commandName)
		{
			int y1 = item->m_yPos + m_top + 7;
			int y2 = y1 + ySize;
			if (g_input.mouseY >= y1 && g_input.mouseY < y2)
			{
				return i;
			}
		}
	}

	return -1;
}


MenuItem *Menu::FindMenuItemByHotkey(char c)
{
	c = tolower(c);
	for (int i = 0; i < m_items.Size(); ++i)
	{
		MenuItem *item = m_items[i];
		if (item->m_hotKeyIndex >= 0)
		{
			char hotkey = tolower(item->m_label[item->m_hotKeyIndex]);
			if (hotkey == c)
			{
				return item;
			}
		}
	}

	return NULL;
}


void Menu::CalculateScreenPostions()
{
	int y = 0;
	int itemHeight = g_propFont->charHeight + MENU_SPACING_Y;

	for (int i = 0; i < m_items.Size(); ++i)
	{
		MenuItem *item = m_items[i];

		item->m_yPos = y;
		if (item->m_objectName && item->m_commandName)
		{
			y += itemHeight;
		}
		else
		{
			// Must be a separator
			y += itemHeight / 2;
		}
	}
}


void Menu::SetRect(int x/* =-1 */, int y/* =-1 */, int w/* =-1 */, int h/* =-1 */)
{
    m_left = x;
	m_top = y;
	CalculateScreenPostions();
    m_width = GetWidthInPixels();
    m_height = GetHeightInPixels();

    int maxY = g_gui->m_height - m_height;
    if (y > maxY)
    {
        m_top = maxY;
        CalculateScreenPostions();
    }
}



// ****************************************************************************
// Class MenuBar
// ****************************************************************************

MenuBar::MenuBar(Widget *parent)
:	Widget(MENU_BAR_NAME, parent),
	m_highlightedMenu(NULL),
	m_displayed(false),
	m_oldHighlightedWidget(NULL)
{
	m_height = g_propFont->charHeight + 1;
	m_growable = false;
	m_highlightable = false;
    m_contextMenu = NULL;
    m_altState = 0;
}


void MenuBar::Initialise()
{
    LoadConfigFile(CONFIG_FILE_NAME);

	CalculateScreenPositions();
}


void MenuBar::LoadConfigFile(char const *filename)
{
	TextFileReader in(filename);

	ReleaseAssert(in.IsOpen(), "Couldn't open '%s'", filename);

	while (in.ReadLine())
	{
		if (in.m_line[0] == '\0') continue;

		MenuItem *item = new MenuItem;
		Menu *menu = NULL;
		MenuBar *menuBar = (MenuBar*)g_gui->GetWidgetByName(MENU_BAR_NAME);

		while (in.TokenAvailable())
		{
			char *token = in.GetNextToken();

			if (StringStartsWith(token, "menu="))
			{
				token += strlen("menu=");
				menu = menuBar->FindMenuByName(token);

				if (!menu)
				{
					menu = new Menu(token, this);
					menuBar->AddMenu(menu);
				}
			}
			else if (StringStartsWith(token, "label="))
			{
				token += strlen("label=");
				if (stricmp(token, "separator") == 0)
				{
					item->m_label = "";
				}
				else
				{
					int len = strlen(token);
					if (token[0] == '"' && token[len-1] == '"')
					{
						token[len-1] = '\0';
						item->m_label = StringDuplicate(token + 1);
					}
					else
					{
						item->m_label = StringDuplicate(token);
					}
				}
			}
			else if (StringStartsWith(token, "key="))
			{
				ReleaseAssert(item->m_label != NULL, "MenuItem key must be defined after label\nIn file '%s'\nAt line %d", in.GetFilename(), in.m_lineNum);
				token += strlen("key=");
				item->m_hotKeyIndex = atoi(token);
				ReleaseAssert(item->m_hotKeyIndex >= -1 && item->m_hotKeyIndex < (int)strlen(item->m_label),
					"MenuItem key character must be a valid index into the label\nIn file '%s'\nAt line %d", in.GetFilename(), in.m_lineNum);
			}
			else if (StringStartsWith(token, "object="))
			{
				token += strlen("object=");
				item->m_objectName = StringDuplicate(token);
			}
			else if (StringStartsWith(token, "command="))
			{
				token += strlen("command=");
				item->m_commandName = StringDuplicate(token);
			}
			else
			{
                ReleaseAssert(item->m_arguments == NULL, "Bad menu definition - too many arguments\nIn file '%s'\nAt line %d", in.GetFilename(), in.m_lineNum);
				item->m_arguments = StringDuplicate(token);
			}
		}

		if (item->m_label)
		{
			ReleaseAssert((int)menu, "MenuItem specified without stating which menu it belongs to - on line");
			if (item->m_hotKeyIndex >= 0)
			{
				MenuItem *existingItem = menu->FindMenuItemByHotkey(item->m_label[item->m_hotKeyIndex]);
				if (existingItem)
				{
                    if (item->m_hotKeyIndex >= 1)
                    {
                        DebugOut("Two %s menu items found with same hotkey: %s and %s use %c\n",
                            menu->m_name, item->m_label, existingItem->m_label,
                            item->m_label[item->m_hotKeyIndex]);
                    }
					item->m_hotKeyIndex = -1;
				}
			}

			menu->AddItem(item);
		}
		else
		{
			delete item;
		}
	}
}


void MenuBar::AddMenu(Menu *menu)
{
	KeyboardShortcut shortcut;
	shortcut.m_commandName = menu->m_name;
	shortcut.m_key = menu->m_name[0];
	shortcut.m_keyQualifiers = KeyboardShortcut::ALT;
	shortcut.m_objectName = "MenuBar";
	shortcut.m_focusRequired = "global";
	g_keyboardShortcutManager->AddShortcut(shortcut);
	m_menus.PutData(menu);
}


char *MenuBar::ExecuteCommand(char const *object, char const *command, char const *arguments)
{
	if (stricmp(command, "LooseFocus") == 0)
	{
		ClearAllState();
		return NULL;
	}
	else
	{
		Menu *menu = FindMenuByName(command);
		if (!menu)
		{
			return NULL;
		}
		m_highlightedMenu = menu;
		menu->m_highlightedItem = 0;
		m_displayed = true;
		GrabFocus();
		return COMMAND_RETURN_NOTHING;
	}
}


Menu *MenuBar::GetMenuTitleUnderMouseCursor()
{
	for (int i = 0; i < m_menus.Size(); ++i)
	{
		Menu *menu = m_menus[i];

		if (menu->IsMouseOverTitle())
		{
			return menu;
		}
	}

	return NULL;
}


void MenuBar::ShowContextMenu(Menu *contextMenu)
{
    m_contextMenu = contextMenu;
    m_contextMenu->m_highlightedItem = 0;
    GrabFocus();
}


void MenuBar::Advance()
{
    if (g_input.keys[KEY_ALT])
    {
        if (m_altState == 0)
            m_altState = 1;
        else
		{
            int numKeyDowns = g_input.numKeyDowns;
			if (g_input.keyDowns[KEY_ALT])
				numKeyDowns--;
			if (numKeyDowns || g_input.lmbClicked ||
				 g_input.mmbClicked || g_input.rmbClicked)
                m_altState = 2;
		}
    }
    else
	{
		if (g_input.keys[KEY_SHIFT] || g_input.keys[KEY_CONTROL])
			ClearMenuBarState();
	}

	if (!g_input.windowHasFocus)
    {
        // Clear the sticky alt key state if shift or control is pressed
        ClearAllState();
    }


	// Menu system can be in one of three states:
	// 1 - no highlighted menu, no displayed menu
	// 2 - highlighted menu, no displayed menu
	// 3 - highlighted menu, displayed menu
	if (!m_highlightedMenu)
	{
		AdvanceNoHighlight();
	}
	else
	{
		AdvanceWithHighlight();
		if (m_highlightedMenu == NULL) return;

		if (m_displayed == false)
		{
			AdvanceHighlightNoDisplay();
		}
		else
		{
			m_highlightedMenu->Advance();
		}
	}

    if (m_contextMenu)
    {
        if (g_input.lmbClicked || g_input.rmbClicked)
        {
            if (!m_contextMenu->IsMouseOver())
            {
                ClearAllState();
            }
        }
        else
        {
            m_contextMenu->Advance();
        }
    }

	if (g_input.keyUps[KEY_ALT])
		m_altState = 0;
}


// Advance with no highlight & no display:
//	If alt, highlight first menu, set keyboard capture
//	If mouse over, highlight relevant menu
void MenuBar::AdvanceNoHighlight()
{
	if (g_input.keyUps[KEY_ALT])
	{
		if (m_altState == 1)
		{
			GrabFocus();
			m_highlightedMenu = m_menus[0];
			return;
		}
	}

	// Is mouse cursor over any of the menu titles?
	Menu *newHighlightedMenu = GetMenuTitleUnderMouseCursor();
	if (newHighlightedMenu)
	{
		g_gui->m_canSleep = false;
		m_highlightedMenu = newHighlightedMenu;
	}
}


// Advance with highlight (common stuff used whether or not a menu is displayed):
//	If mouse over != highlighted, set new highlighted
//	If lmb down
//	  If mouse not over (title or display) or mouse over highlighted title
//		displayed=false, highlighted=NULL
//  If key captured
//	  If key left/right, set new highlighted
//	  If key alt/escape, key capture = false
//  Else
//    If key alt, key capture = true
void MenuBar::AdvanceWithHighlight()
{
	bool const mouseMoved = (abs(g_input.mouseVelX) > 0 ||
							 abs(g_input.mouseVelY) > 0);

	Menu *const menu = GetMenuTitleUnderMouseCursor();
	if (menu && menu != m_highlightedMenu)
        g_gui->m_canSleep = false;

	// Is keyboard active
	if (g_gui->m_focussedWidget == this)
	{
		// Have cursor left/right been pressed?
		int menuIndex = m_menus.FindData(m_highlightedMenu);
		if (g_input.keyDowns[KEY_LEFT]) menuIndex--;
		if (g_input.keyDowns[KEY_RIGHT]) menuIndex++;
		menuIndex += m_menus.Size();
		menuIndex %= m_menus.Size();
		if (m_menus[menuIndex] != m_highlightedMenu)
		{
			m_highlightedMenu = m_menus[menuIndex];
			m_highlightedMenu->m_highlightedItem = 0;
		}

		if (menu != NULL && mouseMoved == true)
		{
			m_highlightedMenu = menu;
		}
	}
	else // Keyboard not captured...
	{
		if (menu != m_highlightedMenu)
		{
			if (m_displayed == false ||	(m_displayed && menu != NULL))
			{
				m_highlightedMenu = menu;
			}
		}
	}

	// Do we want to want to lose key captured or displayed state?
	bool mouseOverDisplayedMenu = false;
	if (m_displayed) mouseOverDisplayedMenu = m_highlightedMenu->IsMouseOver();

	if ((g_gui->m_focussedWidget == this && g_input.keyUps[KEY_ALT] && m_altState == 1) ||
		(g_input.lmbClicked && menu == NULL && !mouseOverDisplayedMenu) ||
		(g_input.lmbClicked && m_displayed && menu == m_highlightedMenu))
	{
		ClearAllState();
		return;
	}

    if (g_input.keyUps[KEY_ALT] &&
        m_altState == 1 &&
        g_gui->m_focussedWidget != this)
	{
		GrabFocus();
		m_highlightedMenu = m_menus[0];
	}
}


// Advance with menu highlighted but not displayed
//	If lmb down
//		if mouse over highlighted, display
//	If enter pressed, display
//  If key typed, choose menu and display
void MenuBar::AdvanceHighlightNoDisplay()
{
	if (g_input.lmbClicked)
	{
		Menu *menu = GetMenuTitleUnderMouseCursor();
		if (menu)
		{
			m_displayed = true;
            menu->m_highlightedItem = -1;
			GrabFocus();
		}
	}

	// Select a menu to display if a key has been pressed
    if (g_gui->m_focussedWidget == this)
    {
        for (int i = 0; i < g_input.numKeysTyped; i++)
	    {
		    Menu *menu = FindMenuByHotKey(g_input.keysTyped[i]);
		    if (menu)
		    {
			    m_highlightedMenu = menu;
			    m_displayed = true;
			    menu->m_highlightedItem = 0;
			    return;
		    }
	    }
    }

	// Have cursor up/down or return been pressed?
	if (g_input.keyDowns[KEY_UP] || g_input.keyDowns[KEY_DOWN] || g_input.keyDowns[KEY_ENTER])
	{
		m_displayed = true;
		GrabFocus();
		return;
	}
}


void MenuBar::GrabFocus()
{
	if (g_gui->m_focussedWidget != this)
	{
		m_oldHighlightedWidget = g_gui->m_focussedWidget;
		g_gui->m_focussedWidget = this;
	}
}


void MenuBar::ClearMenuBarState()
{
	if (g_gui->m_focussedWidget == this)
	{
		g_gui->m_focussedWidget = m_oldHighlightedWidget;
	}

    if (g_gui->m_cursorManager.m_captured == this ||
        g_gui->m_cursorManager.m_captured == m_contextMenu ||
        g_gui->m_cursorManager.m_captured == m_highlightedMenu)
	{
		g_gui->m_cursorManager.m_captured = NULL;
	}

    m_displayed = false;
    m_highlightedMenu = NULL;
}


void MenuBar::ClearAllState()
{
    ClearMenuBarState();
    m_contextMenu = NULL;
}


void MenuBar::CalculateScreenPositions()
{
	int pitch = 3 + MENU_SPACING_Y + g_propFont->charHeight;

	int x = 10;
	for (int i = 0; i < m_menus.Size(); ++i)
	{
		Menu *menu = m_menus[i];

		menu->m_left = x;
		x += GetTextWidth(g_propFont, menu->m_name);
		menu->m_right = x;

		menu->SetRect(menu->m_left, m_top + pitch, menu->m_height, menu->m_width);
		menu->CalculateScreenPostions();

		x += MENU_SPACING_X;
	}
}


void MenuBar::Render()
{
	int y = m_top;

//	ClipRect cr(0, 0, SCREEN_W, SCREEN_H); // TODO

//	bool highlighted = g_guiManager->m_focussedWidget == this;

	// Set mouse pointer to CursorMain if it is over the menu bar
	if (g_input.mouseY < m_height)
	{
		g_gui->m_cursorManager.RequestCursorType(CursorManager::CursorMain);
	}

	// Draw the menu bar background
	DrawFilledBox(m_left, m_top, m_width, m_height, g_gui->m_frameColour2);

    // Draw the selection box around the highlighted menu
    if (m_highlightedMenu)
    {
        DrawFilledBox(m_highlightedMenu->m_left - HIGHLIGHT_BOX_BORDER, 2,
            (m_highlightedMenu->m_right - m_highlightedMenu->m_left) + HIGHLIGHT_BOX_BORDER * 2, m_height + 3,
            g_gui->m_frameColour4);
    }

    // Draw the menu titles
	for (int i = 0; i < m_menus.Size(); ++i)
	{
		Menu *menu = m_menus[i];
		DfColour underlineColour = g_gui->m_textColourFrame;

		// Title text
        DrawTextSimple(g_propFont, g_gui->m_textColourNormal, g_window->bmp, menu->m_left, 3, menu->m_name);

        // Underline first character of title text
		int width = GetTextWidth(g_propFont, menu->m_name, 1);
		DrawHLine(menu->m_left, g_propFont->charHeight + 2, width, underlineColour);
	}

	// Draw the displayed menu
	if (m_displayed)
	{
//		m_highlightedMenu->SetRect(m_highlightedMenu->m_left - HIGHLIGHT_BOX_BORDER, m_height + 1);
//		m_highlightedMenu->m_top = 100;
		m_highlightedMenu->Render();
	}

    if (m_contextMenu)
        m_contextMenu->Render();

	// Render the FPS meter
    DrawTextRight(g_propFont, g_gui->m_textColourFrame, g_window->bmp, g_window->bmp->width - 5, y, "FPS: %d", g_window->fps);
}


Menu *MenuBar::FindMenuByName(char const *name)
{
	for (int i = 0; i < m_menus.Size(); ++i)
	{
		if (stricmp(name, m_menus[i]->m_name) == 0) return m_menus[i];
	}

	return NULL;
}


Menu *MenuBar::FindMenuByHotKey(char c)
{
	for (int i = 0; i < m_menus.Size(); ++i)
	{
		if (tolower(c) == tolower(m_menus[i]->m_name[0]))
		{
			return m_menus[i];
		}
	}

	return NULL;
}


bool MenuBar::DoesClickHitMenu()
{
    bool hitFound = false;

	if (m_highlightedMenu)
        hitFound = m_highlightedMenu->IsMouseOver();
    if (m_contextMenu)
        hitFound |= m_contextMenu->IsMouseOver();

    return hitFound;
}
