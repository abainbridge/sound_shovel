#pragma once


#include "containers/llist.h"


// ****************************************************************************
// Class KeyboardShortcut
// ****************************************************************************

class KeyboardShortcut
{
public:
	enum	// Key qualifiers bit masks
	{
		SHIFT	= 1,
		ALT		= 2,
		CTRL	= 4
	};

	enum	// Flag bit masks
	{
		UP_EVENT	 = 1,  // Mark shortcut as applying on the up event rather than the down event as is normal
        PASS_THROUGH = 2,  // Mark shortcut as not grabbing the key event for itself - instead allow the event to be seen by other parts of the program too
    };

	unsigned char	m_key;
	unsigned char	m_keyQualifiers;
	unsigned char	m_flags;
	char const		*m_objectName;		// Name of object to send message to
	char const		*m_commandName;
	char const		*m_arguments;
	char const		*m_focusRequired;	// Object that must have focus before the shortcut will be recognized. Normally the same as m_objectName

public:
	KeyboardShortcut();
	bool IsActiveThisFrame(unsigned char qualifierFlags);	// Returns true if this keyboard shortcut should be activated this frame
	char const *GetQualifierString();
};


// ****************************************************************************
// Class KeyboardShortcutManager
// ****************************************************************************

class KeyboardShortcutManager
{
private:
	LList <KeyboardShortcut> m_shortcuts;
    char *m_userConfigFilename;

private:
	bool ParseKey(KeyboardShortcut *shortcut, char *token);
	void LoadConfigFile(char const *filename);

public:
	KeyboardShortcutManager(char const *configFilename);

    char const *GetUserConfigFilename() { return m_userConfigFilename; }

	void AddShortcut(KeyboardShortcut &shortcut);
	void Advance();
	KeyboardShortcut const *LookupShortcut(char const *object, char const *command, char const *arguments);
};


extern KeyboardShortcutManager *g_keyboardShortcutManager;
