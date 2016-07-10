// Own header
#include "keyboard_shortcuts.h"

#include "andy_string.h"
#include "filesys_utils.h"
#include "string_utils.h"
//#include "system_info.h"
#include "text_stream_readers.h"
#include "gui/command.h"
#include "gui/gui_manager.h"

// Contrib headers
#include "df_input.h"

// Standard headers
#include <ctype.h>
#include <stdlib.h>	// For atoi
#include <string.h>


// ****************************************************************************
// Class KeyboardShortcut
// ****************************************************************************

KeyboardShortcut::KeyboardShortcut()
:	m_key(0),
	m_keyQualifiers(0),
	m_flags(0),
	m_objectName(NULL),
	m_commandName(NULL),
	m_arguments(NULL),
	m_focusRequired(NULL)
{
}


bool KeyboardShortcut::IsActiveThisFrame(unsigned char qualifierFlags)
{
	if (g_inputManager.keyDowns[m_key])
	{
		if (m_key == KEY_SHIFT || m_key == KEY_ALT || m_key == KEY_CONTROL)
		{
			return true;
		}
		else if (qualifierFlags == m_keyQualifiers)
		{
			return true;
		}
	}

	return false;
}


char const *KeyboardShortcut::GetQualifierString()
{
	static char buf[sizeof("Shift+Alt+Ctrl") + 1];
	buf[0] = '\0';
	if (m_keyQualifiers & SHIFT)
		strcat(buf, "Shift");
	if (m_keyQualifiers & ALT)
		strcat(buf, "+Alt");
	if (m_keyQualifiers & CTRL)
		strcat(buf, "+Ctrl");

	if (buf[0] == '+')
		return buf + 1;
	return buf;
}


// ****************************************************************************
// Class KeyboardShortcutManager
// ****************************************************************************

KeyboardShortcutManager *g_keyboardShortcutManager = NULL;


KeyboardShortcutManager::KeyboardShortcutManager(char const *configFilename)
{
	LoadConfigFile(configFilename);
}


// Returns false on failure
bool KeyboardShortcutManager::ParseKey(KeyboardShortcut *shortcut, char *token)
{
	while (!IsSpace(token[0]) && token[0] != '\0')
	{
        if (StringStartsWith(token, "Ctrl+"))
        {
            token += 5;
            shortcut->m_keyQualifiers |= KeyboardShortcut::CTRL;
        }
        else if (StringStartsWith(token, "Shift+"))
        {
            token += 6;
            shortcut->m_keyQualifiers |= KeyboardShortcut::SHIFT;
        }
        else if (StringStartsWith(token, "Alt+"))
        {
            token += 4;
            shortcut->m_keyQualifiers |= KeyboardShortcut::ALT;
        }
        else if (stricmp(token, "tab") == 0)
		{
			shortcut->m_key = KEY_TAB;
			token += 3;
		}
		else if (tolower(token[0]) == 'f' && isdigit(token[1]))
		{
			shortcut->m_key = KEY_F1 + atoi(&token[1]) - 1;
			token += 2;
			if (!IsSpace(token[0]) && token[0] != '\0') token++;
		}
		else
        {
			int keyId = GetKeyId(token);
            if (keyId == -1) return false;
			shortcut->m_key = keyId;
			token++;
			while (isalnum(token[0])) token++;
        }
	}

	return true;
}


void KeyboardShortcutManager::LoadConfigFile(char const *filename)
{
	TextFileReader in(filename);

	ReleaseAssert(in.IsOpen(), "Couldn't open '%s'", filename);

	while (in.ReadLine())
	{
		if (in.m_line[0] == '\0') continue;

		KeyboardShortcut shortcut;
		while (in.TokenAvailable())
		{
			char *token = in.GetNextToken();

			if (StringStartsWith(token, "key="))
			{
				token += 4;
				bool rv = ParseKey(&shortcut, token);
	            ReleaseAssert(rv, "Keyboard shortcut - invalid key spec\nIn file '%s'\nAt line %d", in.GetFilename(), in.m_lineNum);
			}
			else if (StringStartsWith(token, "object="))
			{
				token += 7;
				if (token[0] == '"') token++;
                ReleaseAssert(!IsSpace(token[0]), "Keyboard shortcut - invalid object spec\nIn file '%s'\nAt line %d", in.GetFilename(), in.m_lineNum);
                int lastChar = strlen(token) - 1;
				if (token[lastChar] == '"') token[lastChar] = '\0';
				shortcut.m_objectName = StringDuplicate(token);
			}
			else if (StringStartsWith(token, "command="))
			{
				token += 8;
                ReleaseAssert(!IsSpace(token[0]), "Keyboard shortcut - invalid command spec\nIn file '%s'\nAt line %d", in.GetFilename(), in.m_lineNum);
				shortcut.m_commandName = StringDuplicate(token);
			}
			else if (StringStartsWith(token, "flags="))
			{
				token += 6;
                ReleaseAssert(!IsSpace(token[0]), "Keyboard shortcut - invalid flags spec\nIn file '%s'\nAt line %d", in.GetFilename(), in.m_lineNum);
                if (stricmp(token, "passThrough") == 0)
                {
                    shortcut.m_flags |= KeyboardShortcut::PASS_THROUGH;
                }
				else
				{
					ReleaseAssert(0, "Keyboard shortcut - unrecognized flag\nIn file '%s'\nAt line %d", in.GetFilename(), in.m_lineNum);
				}
			}
			else if (StringStartsWith(token, "focus="))
			{
				token += 6;
				shortcut.m_focusRequired = StringDuplicate(token);
			}
			else
			{
                ReleaseAssert(shortcut.m_arguments == NULL, "Keyboard shortcut - too many arguments\nIn file '%s'\nAt line %d", in.GetFilename(), in.m_lineNum);
				shortcut.m_arguments = StringDuplicate(token);
			}
		}

		// Make sure we have a complete shortcut definition now
		ReleaseAssert(shortcut.m_key != 0, "Keyboard shortcut - invalid key spec\nIn file '%s'\nAt line line %d", in.GetFilename(), in.m_lineNum);
		ReleaseAssert(shortcut.m_objectName != NULL, "Keyboard shortcut - invalid object spec\nIn file '%s'\nAt line line %d", in.GetFilename(), in.m_lineNum);
		ReleaseAssert(shortcut.m_commandName != NULL, "Keyboard shortcut - invalid command spec\nIn file '%s'\nAt line line %d", in.GetFilename(), in.m_lineNum);

		// Copy object name into focusRequired if not already specified
		if (!shortcut.m_focusRequired)
			shortcut.m_focusRequired = StringDuplicate(shortcut.m_objectName);

		m_shortcuts.PutData(shortcut);
	}
}


void KeyboardShortcutManager::AddShortcut(KeyboardShortcut &shortcut)
{
	// Copy object name into focusRequired if not already specified
	if (!shortcut.m_focusRequired)
		shortcut.m_focusRequired = StringDuplicate(shortcut.m_objectName);

	// Make sure specified shortcut doesn't clash with an existing one

	for (int i = 0; i < m_shortcuts.Size(); ++i)
	{
		KeyboardShortcut *a = m_shortcuts.GetPointer(i);

		// Does this shortcut use same key chord as the new one
		if (a->m_key == shortcut.m_key && a->m_keyQualifiers == shortcut.m_keyQualifiers)
		{
			// Yes it does. Maybe it doesn't clash due to different scoping rules
			if (stricmp(a->m_focusRequired, shortcut.m_focusRequired) == 0)
			{
				ReleaseWarn(a->m_flags & KeyboardShortcut::PASS_THROUGH,
						"Two keyboard shortcut actions registered for the same key combination.\n"
						"1: %s %s\n"
						"2: %s %s\n"
						"Key combination is %s+%s",
						shortcut.m_objectName, shortcut.m_commandName,
						a->m_objectName, a->m_commandName,
						a->GetQualifierString(), GetKeyName(a->m_key));
			}
		}
	}

	m_shortcuts.PutData(shortcut);
}


void KeyboardShortcutManager::Advance()
{
	unsigned char qualifierFlags = 0;

	if (g_inputManager.keys[KEY_SHIFT]) qualifierFlags |= KeyboardShortcut::SHIFT;
	if (g_inputManager.keys[KEY_CONTROL]) qualifierFlags |= KeyboardShortcut::CTRL;
	if (g_inputManager.keys[KEY_ALT]) qualifierFlags |= KeyboardShortcut::ALT;

	// Go through all the shortcuts once. If they should be active,
	// and they are to be sent to a widget and that widget has
	// focus, then send them to that widget.
	for (int i = 0; i < m_shortcuts.Size(); ++i)
	{
		KeyboardShortcut *shortcut = m_shortcuts.GetPointer(i);

		if (shortcut->IsActiveThisFrame(qualifierFlags))
		{
			if (stricmp(shortcut->m_focusRequired, "global") == 0)
				continue;

			// Is the focused widget, or one of its encapsulated children an acceptable
			// match for the required focus? eg the if the find dialog is focused it is
			// an acceptable match for edit box commands because it has more than zero
			// edit boxes. (We use this to give the find dialog a chance to send the command
			// to the appropriate edit box.)
			Widget *focusMatch = g_guiManager->m_focussedWidget->GetWidgetByName(shortcut->m_focusRequired);
			if (focusMatch)
			{
                if (!(shortcut->m_flags & KeyboardShortcut::PASS_THROUGH))
                {
                    g_inputManager.keyDowns[shortcut->m_key] = 0;
                    UntypeKey(shortcut->m_key);
                }

				// Send the command...
				g_commandSender.SendCommandNoRV(
					"Keyboard shortcut",
					shortcut->m_objectName,
					shortcut->m_commandName,
					shortcut->m_arguments);
			}
		}
	}

	// Go through all the shortcuts again. If they should be active,
	// and weren't sent in the pass above, then send them.
	for (int i = 0; i < m_shortcuts.Size(); ++i)
	{
		KeyboardShortcut *shortcut = m_shortcuts.GetPointer(i);

		if (shortcut->IsActiveThisFrame(qualifierFlags))
		{
			if (stricmp(shortcut->m_focusRequired, "global") == 0)
			{
				g_commandSender.SendCommandNoRV(
					"KeyboardShortcutManager",
					shortcut->m_objectName,
					shortcut->m_commandName,
					shortcut->m_arguments);
			}
		}
	}

    // Now send 'normal' keypresses that the user has just typed
	for (int i = 0; i < g_inputManager.numKeysTyped; ++i)
	{
        char key[] = { g_inputManager.keysTyped[i], '\0' };
        g_commandSender.SendCommandNoRV("KeyboardShortcutManager",
                                        g_guiManager->m_focussedWidget->m_name,
                                        "insert_text",
                                        key);
	}
}


KeyboardShortcut const *KeyboardShortcutManager::LookupShortcut(
	char const *object, char const *command, char const *arguments)
{
	for (int i = 0; i < m_shortcuts.Size(); i++)
	{
		KeyboardShortcut *s = m_shortcuts.GetPointer(i);
		if (stricmp(command, s->m_commandName) == 0 &&
			stricmp(object, s->m_objectName) == 0)
		{
			if (arguments)
			{
				if (s->m_arguments && stricmp(arguments, s->m_arguments) == 0)
					return s;
			}
			else
			{
				if (!s->m_arguments)
					return s;
			}
		}
	}

	return NULL;
}
