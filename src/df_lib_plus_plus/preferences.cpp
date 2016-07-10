// Own header
#include "preferences.h"

// Contrib headers
#include "df_common.h"

// Project headers
#include "andy_string.h"
#include "filesys_utils.h"
#include "string_utils.h"

// Standard headers
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>


PrefsManager *g_prefs = NULL;


// *** Class PrefsItem ********************************************************

PrefsItem::PrefsItem()
:	m_type(TypeUnknown),
	m_registered(false),
	m_variesWithSyntaxGroup(false)
{
}


// *** Class PrefsItemString **************************************************

PrefsItemString::PrefsItemString(char const *_str)
:	PrefsItem()
{
	m_type = TypeString;

	if (_str && stricmp(_str, "null") != 0)
		m_str = StringDuplicate(_str);
	else
		m_str = NULL;
}


PrefsItemString::~PrefsItemString()
{
	delete[] m_str;
}


void PrefsItemString::WriteValIntoBuf(char *buf, unsigned bufSize)
{
	if (m_str)
		_snprintf(buf, bufSize, "%s", m_str);
	else
		_snprintf(buf, bufSize, "null");
}


bool PrefsItemString::SetValueFromLine(char const *line)
{
	if (m_str)
		delete[] m_str;
	if (!line || stricmp(line, "null") == 0)
		m_str = NULL;
	else
		m_str = StringDuplicate(line);

	return true;
}


// *** Class PrefsItemFloat ****************************************************

PrefsItemFloat::PrefsItemFloat(float _float)
:	PrefsItem()
{
	m_type = TypeFloat;
	m_float = _float;
}


void PrefsItemFloat::WriteValIntoBuf(char *buf, unsigned bufSize)
{
	_snprintf(buf, bufSize, "%.2f", m_float);
}


bool PrefsItemFloat::SetValueFromLine(char const *line)
{
	m_float = atof(line);
// 	if (m_float > m_max)
// 	{
// 		m_float = m_max;
// 		return false;
// 	}
// 	else if (m_float < m_min)
// 	{
// 		m_float = m_min;
// 		return false;
// 	}

	return true;
}


// *** Class PrefsItemInt *****************************************************

PrefsItemInt::PrefsItemInt(int _int, int _min, int _max)
:	PrefsItem()
{
	m_type = TypeInt;
	m_int = _int;
	m_min = _min;
	m_max = _max;
}


void PrefsItemInt::WriteValIntoBuf(char *buf, unsigned bufSize)
{
	_snprintf(buf, bufSize, "%d", m_int);
}


bool PrefsItemInt::SetValueFromLine(char const *line)
{
	m_int = atoi(line);
	if (m_int > m_max)
	{
		m_int = m_max;
		return false;
	}
	else if (m_int < m_min)
	{
		m_int = m_min;
		return false;
	}

	return true;
}



// ****************************************************************************
// Class PrefsManager
// ****************************************************************************

PrefsManager::PrefsManager()
{
    m_filename = NULL;
}


bool PrefsManager::IsLineEmpty(char const *line)
{
	while (line[0] != '\0')
	{
		if (line[0] == '#') return true;
		if (isalnum(line[0])) return false;
		++line;
	}

	return true;
}


void PrefsManager::AddLine(char *line)
{
	m_fileText.PutData(StringDuplicate(line));

    char localCopy[256];
    strncpy(localCopy, line, 255);
	if (!IsLineEmpty(localCopy))				// Skip comment lines and blank lines
	{
		// Skip leading whitespace
		char *key = localCopy;
		while (!isalnum(*key) && *key != '\0')
			key++;

		// Find end of key
		char *c = key;
		while (!IsSpace(*c))			// Find the end of the key word
			c++;
		if (*c == '\0')
			goto handle_error;

		*c = '\0';

		// Find value
		c++;
		while (IsSpace(*c))
		{
			c++;
			if (*c == '\0')
				goto handle_error;
		}
		if (*c != '=')
			goto handle_error;
		c++;
		while (IsSpace(*c))
		{
			c++;
			if (*c == '\0')
				goto handle_error;
		}

		// Strip trailing newline
		c[strlen(c) - 1] = '\0';

		PrefsItem *item = m_items.GetData(key);
		if (strchr(key, '.'))
 		{
			if (!item)
			{
				item = CreateOverrideItem(key);
				m_items.PutData(key, item);
			}

 			if (!item->SetValueFromLine(c))
 				ReleaseWarn(false, "Local settings file contained out of range value:%s\n", line);
		}
		else
		{
			ReleaseWarn((int)item, "Local settings file contains an unrecognized setting:%s\n", line);

			if (item)
				item->SetValueFromLine(c);
		}
	}

	return;

handle_error:
	ReleaseWarn(false, "Local settings file contained bad line:\n", line);
}


void PrefsManager::Load(char const *filename)
{
    m_filename = StringDuplicate(filename);

    // Try to read preferences if they exist
    FILE *in = fopen(m_filename, "r");

    if (in)
    {
        struct _stat statBuf;
        _stat(m_filename, &statBuf);
        m_diskFileModTimeWhenRead = statBuf.st_mtime;

        char line[256];
	    while (fgets(line, 255, in) != NULL)
            AddLine(line);

		fclose(in);
    }
}


void PrefsManager::SaveItem(FILE *out, char const *key, PrefsItem *_item)
{
	// Is this item language specific?
	if (strchr(key, '.') == NULL)
	{
		// No it isn't, so don't save unregistered items
		if (!_item->m_registered)
			return;
	}

	char buf[128];
	_item->WriteValIntoBuf(buf, sizeof(buf));
	fprintf(out, "%s = %s\n", key, buf);

	_item->m_hasBeenWritten = true;
}


void PrefsManager::Save()
{
    // We've got a copy of the plain text from the prefs file that we initially
	// loaded in the variable m_fileText. We use that file as a template with
	// which to create the new save file. Updated prefs items values are looked up
	// as it their turn to be output comes around. When we've finished we then
	// write out all the new prefs items because they didn't exist in m_fileText.

	// First clear the "has been written" flags on all the items
	for (unsigned int i = 0; i < m_items.Size(); ++i)
	{
		if (m_items.ValidIndex(i))
			m_items[i]->m_hasBeenWritten = false;
	}

	// Now use m_fileText as a template to write most of the items
    String newFilename = m_filename + String(".tmp");
    FILE *out = fopen(newFilename.c_str(), "w");

	// If we couldn't open the prefs file for writing then just silently fail -
	// it's better than crashing.
	if (!out)
		return;

	// For each line in the saved copy of the config file...
	for (int i = 0; i < m_fileText.Size(); ++i)
	{
		char const *line = m_fileText[i];
		if (IsLineEmpty(line))
		{
			fprintf(out, "%s", line);
		}
		else
		{
			// This line isn't blank. Search for the start of a key name
			char const *c = line;
			char const *keyStart = NULL;
			char const *keyEnd = NULL;
			while (*c != '\0')
			{
				if (isalnum(*c) || *c == '.')
				{
					keyStart = c;
					break;
				}
				++c;
			}

			if (!keyStart)
			{
				// Don't understand this line, just copy it to the output
				fprintf(out, "%s", line);
				continue;
			}

			// Search for the end of the key name
			while(*c != '\0')
			{
				if (!isalnum(*c) && *c != '.')
				{
					keyEnd = c;
					break;
				}
				c++;
			}

			if (!keyEnd)
			{
				// Don't understand this line, just copy it to the output
				fprintf(out, "%s", line);
				continue;
			}

			char key[128];
			int keyLen = keyEnd - keyStart;
			strncpy(key, keyStart, keyLen);
			key[keyLen] = '\0';
			PrefsItem *item = m_items.GetData(key);
			if (item && !item->m_hasBeenWritten)
				SaveItem(out, key, item);
		}
	}

	// Finally output any items that haven't already been written
	for (unsigned int i = 0; i < m_items.Size(); ++i)
	{
		if (m_items.ValidIndex(i))
		{
			PrefsItem *item = m_items.GetData(i);
			char const *key = m_items.GetName(i);
			if (!item->m_hasBeenWritten)
			{
				SaveItem(out, key, item);
			}
		}
	}

	fclose(out);

//     struct _stat statBuf;
//     if (_stat(m_filename, &statBuf) != -1)
//     {
//         if (statBuf.st_mtime != m_diskFileModTimeWhenRead)
//         {
//             if (AreFilesIdentical(newFilename.c_str(), m_filename))
//                 return;
//             else
//             {
//                 String messageText = "The configuration file:\n\n";
//                 messageText += m_filename;
//                 messageText += "\n\nhas been modified since it was loaded.\n\nDo you want to overwrite it?";
//                 int result = MessageDialog(APPLICATION_NAME, messageText.c_str(), MsgDlgTypeYesNo);
//                 if (result != MsgDlgRtnCode_Ok)
//                 {
//                     return;
//                 }
//             }
//         }
//     }

    bool result = MoveFile_(newFilename.c_str(), m_filename);
    DebugAssert(result);
}


void PrefsManager::SetString(char const *_key, char const *_string)
{
	int index = m_items.GetIndex(_key);

	if (index == -1)
	{
		PrefsItemString *item = new PrefsItemString(_string);
		m_items.PutData(_key, item);
	}
	else
	{
		PrefsItem *item = m_items.GetData(index);
		DebugAssert(item->m_type == PrefsItem::TypeString);
		PrefsItemString *stringItem = (PrefsItemString*)item;
		char *newString = StringDuplicate(_string);
        delete [] stringItem->m_str;
		stringItem->m_str = newString;
	}
}


void PrefsManager::SetFloat(char const *_key, float _float)
{
	int index = m_items.GetIndex(_key);

	if (index == -1)
	{
		PrefsItemFloat *item = new PrefsItemFloat(_float);
		m_items.PutData(_key, item);
	}
	else
	{
		PrefsItem *item = m_items.GetData(index);
		DebugAssert(item->m_type == PrefsItem::TypeFloat);
		PrefsItemFloat *floatItem = (PrefsItemFloat*)item;
		floatItem->m_float = _float;
	}
}


void PrefsManager::SetInt(char const *_key, int _int)
{
	int index = m_items.GetIndex(_key);

	if (index == -1)
	{
		PrefsItemInt *item = new PrefsItemInt(_int, 0, 0);
		m_items.PutData(_key, item);
	}
	else
	{
		PrefsItem *item = m_items.GetData(index);
		DebugAssert(item->m_type == PrefsItem::TypeInt);
		PrefsItemInt *intItem = (PrefsItemInt*)item;
		intItem->m_int = _int;
	}
}


// Lets say key is tabSize and lang is Python. If so, we look for:
// 1. m_items->GetData("TabSize.Python")
// 2. lang->m_prefsItems->GetData("TabSize")
// 3. m_items->GetData("TabSize")
// We return the first one we find
PrefsItem *PrefsManager::GetItem(char const *key) const
{
	PrefsItem *item = NULL;

#if _DEBUG
	// Check that the requested key is registered
	item = m_items.GetData(key);
	DebugAssert(item);
	DebugAssert(item->m_registered);
	// if (item->m_variesWithSyntaxGroup) DebugAssert(lang); // TODO To enable this assert, create a NullSyntaxHighlighter, so that callers that really want the global default can request it.
	item = NULL;
#endif

	// If still not found, look for Trowel wide setting (this will
	// retrieve the user preference, if there is one, or the trowel
	// default that was registered at startup.
	if (!item)
		item = m_items.GetData(key);

	// If still not found, that's an error
	ReleaseAssert((int)item, "PrefsManager::GetString: '%s' not found", key);

	return item;
}


char *PrefsManager::GetString(char const *key) const
{
	PrefsItem *item = GetItem(key);
	DebugAssert(item->m_type == PrefsItem::TypeString);
	PrefsItemString *strItem = (PrefsItemString*)item;
	return strItem->m_str;
}


float PrefsManager::GetFloat(char const *key) const
{
	PrefsItem *item = GetItem(key);
	DebugAssert(item->m_type == PrefsItem::TypeFloat);
	PrefsItemFloat *floatItem = (PrefsItemFloat*)item;
	return floatItem->m_float;
}


int PrefsManager::GetInt(char const *key) const
{
	PrefsItem *item = GetItem(key);
	DebugAssert(item->m_type == PrefsItem::TypeInt);
	PrefsItemInt *intItem = (PrefsItemInt*)item;
	return intItem->m_int;
}


void PrefsManager::RegisterInt(char const *name, int _default, int _min, int _max, bool variesWithSyntaxGroup)
{
	DebugAssert(!m_items.GetData(name));
	PrefsItem *item = new PrefsItemInt(_default, _min, _max);
	m_items.PutData(name, item);
	item->m_registered = true;
	item->m_variesWithSyntaxGroup = variesWithSyntaxGroup;
}


void PrefsManager::RegisterFloat(char const *name, float _default, bool variesWithSyntaxGroup)
{
	DebugAssert(!m_items.GetData(name));
	PrefsItem *item = new PrefsItemFloat(_default);
	m_items.PutData(name, item);
	item->m_registered = true;
	item->m_variesWithSyntaxGroup = variesWithSyntaxGroup;
}


void PrefsManager::RegisterString(char const *name, char const *_default, bool variesWithSyntaxGroup)
{
	DebugAssert(!m_items.GetData(name));
	PrefsItemString *item = new PrefsItemString(_default);
	m_items.PutData(name, item);
	item->m_registered = true;
	item->m_variesWithSyntaxGroup = variesWithSyntaxGroup;
}


void PrefsManager::GetBaseName(char const *fullName, char *baseName)
{
	char const *c = fullName;
	char *d = baseName;
	while (*c != '.')
	{
		*d = *c;
		c++;
		d++;
	}
	*d = '\0';
}


PrefsItem *PrefsManager::CreateOverrideItem(char const *fullName)
{
	DebugAssert(strlen(fullName) < 256);
	char baseName[256];
	GetBaseName(fullName, baseName);

	PrefsItem *baseItem = m_items.GetData(baseName);
	PrefsItem *item = NULL;

	switch (baseItem->m_type)
	{
		case PrefsItem::TypeFloat:
			item = new PrefsItemFloat(0.0f);
			break;
		case PrefsItem::TypeInt:
		{
			PrefsItemInt *baseItemInt = (PrefsItemInt*)baseItem;
			item = new PrefsItemInt(baseItemInt->m_min, baseItemInt->m_min, baseItemInt->m_max);
			break;
		}
		case PrefsItem::TypeString:
			item = new PrefsItemString(NULL);
			break;
	}

	return item;
}
