#pragma once


// Project headers
#include "widget.h"
#include "containers/llist.h"


#define MENU_BAR_NAME "MenuBar"


// ****************************************************************************
// Class MenuItem
// ****************************************************************************

class MenuItem
{
public:
	char const *m_label;
	char const *m_objectName;
	char const *m_commandName;  // If command is NULL, item is assumed to be a separator
	int			m_hotKeyIndex;	// Index into m_label, indicating which character is the keyboard hot-key for that item
	char	   *m_shortcut;		// Contains the keyboard shortcut as a displayable string (This is determined at run-time by comparing the command to those already registered in the keyboard shortcuts system)
	char const *m_arguments;
	int			m_yPos;         // Internal use only: Filled out by Menu::CalculateScreenPostions

	MenuItem();
	~MenuItem();
	void Execute();
};


// ****************************************************************************
// Class Menu
// ****************************************************************************

class Menu: public Widget
{
public:
	int						m_left;				// \ Position of left-most and right-most pixels
	int						m_right;			// / occupied by the menu's title in the menu bar
	char const				*m_name;
	LList <MenuItem *>		m_items;
	int						m_longestShortcutLen;	// Length (in pixels) of the longest shortcut display string on this menu
	int						m_highlightedItem;

	Menu(char const *_name, Widget *parent);

	void AddItem(MenuItem *item);

	void Advance();
	void Render();
	void SetRect(int x=-1, int y=-1, int w=-1, int h=-1);

	int GetWidthInPixels();
	int GetHeightInPixels();

	bool IsMouseOverTitle();
	bool IsMouseOver();
	int	 IsMouseOverItem();	                    // Returns the index of the item that the mouse is over, -1 if none

	void CalculateScreenPostions();

	MenuItem *FindMenuItemByHotkey(char c);
};


// ****************************************************************************
// Class MenuBar
// ****************************************************************************

class MenuBar: public Widget
{
	friend class MenuItem;

private:
	LList <Menu *>		m_menus;
	Menu				*m_highlightedMenu;		// NULL if no menu is highlighted
	bool				m_displayed;			// True if the highlighted menu is displayed
	Widget				*m_oldHighlightedWidget;// Used to restore g_guiManager->m_highlightedWidget when we unhighlight ourselves
    Menu                *m_contextMenu;         // Null if there is no current context menu
    int                 m_altState;             // Used to make alt up only focus menubar if no input events have occurred since alt down

private:
	void LoadConfigFile(char const *filename);
	void CalculateScreenPositions();			// Work out position (in pixels) of menu renderables, to save work in each render
	Menu *GetMenuTitleUnderMouseCursor();
	void AdvanceNoHighlight();
	void AdvanceWithHighlight();
	void AdvanceHighlightNoDisplay();
	void GrabFocus();
    void ClearMenuBarState();
    void ClearAllState();						// Call this when user presses ESC or clicks away etc

public:
    MenuBar(Widget *parent);

	void Initialise();

	void AddMenu(Menu *menu);
	char *ExecuteCommand(char const *object, char const *command, char const *arguments);

    void ShowContextMenu(Menu *contextMenu);

    void Advance();
	void Render();

	Menu *FindMenuByName(char const *name);
	Menu *FindMenuByHotKey(char c);
	bool DoesClickHitMenu();
};
