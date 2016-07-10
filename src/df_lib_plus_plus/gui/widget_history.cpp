// Own header
#include "widget_history.h"

// Project headers
#include "string_utils.h"

// Standard headers
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>


WidgetHistory *g_widgetHistory = NULL;


// **************
// Class HistItem
// **************

HistItem::HistItem(char const *_str)
:	m_type(TypeString)
{
	m_str = StringDuplicate(_str);
}


HistItem::HistItem(float _float)
:	m_type(TypeFloat),
	m_float(_float)
{
}


HistItem::HistItem(int _int)
:	m_type(TypeInt),
	m_int(_int)
{
}


HistItem::~HistItem()
{
	delete[] m_str;
}


void HistItem::SetValueFromLine(char const *_line)
{
	// Get value
	char const *value = _line;
	while (IsSpace(*value) || *value == '=')
	{
		if (*value == '\0') break;
		value++;
	}
	
	// Is value a number?
	if (value[0] == '-' || IsDigit(value[0]))
	{
		// Guess that number is an int
		m_type = TypeInt;
		
		// Verify that guess
		char const *c = value;
        int numDots = 0;
		while (*c != '\0')
		{
			if (*c == '.')
			{
                ++numDots;
			}
			else if (!IsDigit(*c) && *c != '-')
			{
				numDots	+= 2;
			}
			++c;
		}
        if (numDots == 1) m_type = TypeFloat;
        else if (numDots > 1) m_type = TypeString;
		
		// Convert string into a real number
		if      (m_type == TypeFloat)	    m_float = atof(value);
        else if (m_type == TypeString)      m_str = StringDuplicate(value);
		else						        m_int = atoi(value);
	}
	else
	{
		m_type = TypeString;

		// Count the number of escape chars
        int unescapedLen = 0;
        for (char const *c = value; *c != '\0'; c++)
        {
            if (*c == '\\')
                c++; // Skip the escaped char in case it is another '\', which we would confuse to loop logic
            unescapedLen++;
        }

		// Allocate a string big enough for the unescaped version
		m_str = new char [unescapedLen + 1];
        
		// Copy the string, unescaping as we go
        char *c = m_str;
        for (int i = 0; value[i] != '\0'; i++)
        {
            if (value[i] == '\\')
            {
                if (value[i + 1] == 'n')
                    *c = '\n';
                else if (value[i + 1] == 'r')
                    *c = '\r';
                i++;
            }
            else
            {
                *c = value[i];
            }

            c++;
        }
        
        *c = '\0';
	}
}



// ******************
// Class WidgetHistory
// ******************

WidgetHistory::WidgetHistory(char const *_filename)
:   CommandReceiver("WidgetHistory")
{
    g_commandSender.RegisterReceiver(this);

    m_filename = StringDuplicate(_filename);

	Load();
}


WidgetHistory::~WidgetHistory()
{
}


bool WidgetHistory::IsLineEmpty(char const *_line)
{
	while (_line[0] != '\0')
	{
		if (_line[0] == '#') return true;
		if (isalnum(_line[0])) return false;
		++_line;
	}

	return true;
}	


char *WidgetHistory::GetKeyFromLine(char *line)
{
	// Skip leading whitespace
	char *key = line;
	while (!isalnum(*key) && *key != '\0')
	{
		key++;
	}

	// Find end of key
	char *c = key;
	while (isalnum(*c))							// Find the end of the key word
	{
		c++;
	}
	*c = '\0';

	// Copy the key and return it
	key = StringDuplicate(key);
	return key;
}


void WidgetHistory::AddLine(char *_line)
{
    char localCopy[256];
    strncpy(localCopy, _line, 255);
	if (!IsLineEmpty(localCopy))				// Skip comment lines and blank lines
	{
		// Remove trailing new line
		char *c = localCopy + strlen(localCopy) - 1;
		if (c[0] == '\n')
			c[0] = '\0';

		// Get key
		char *key = GetKeyFromLine(localCopy);

        // Make sure key doesn't already exist
        if (m_items.GetIndex(key) != -1)
            return;

		c = localCopy + strlen(key) + 1;
		// Create item, set its value and add it to m_items
		HistItem *item = new HistItem;
		item->SetValueFromLine(c);
		m_items.PutData(key, item);
	}  
}


void WidgetHistory::Load(char const *_filename)
{
	if (!_filename) _filename = m_filename;

	m_items.EmptyAndDelete();
    
    // Try to read preferences if they exist
    FILE *in = fopen(_filename, "r");

    if (in)
    {
        struct _stat statBuf;
        _stat(_filename, &statBuf);
        m_diskFileModTimeWhenRead = statBuf.st_mtime;

        char line[256];
	    while (fgets(line, sizeof(line), in) != NULL)
	    {
            // Ignore lines longer than 256, because they are probably corrupt
            int len = strlen(line);
            int maxLen = sizeof(line) - 1;
            if (len < maxLen)
                AddLine(line);
        }
    	fclose(in);
    }
}


void WidgetHistory::SaveItem(FILE *out, char const *key, HistItem *_item)
{
	switch (_item->m_type)
	{
		case HistItem::TypeFloat:
			fprintf(out, "%s = %.2f\n", key, _item->m_float);
			break;
		case HistItem::TypeInt:
			fprintf(out, "%s = %d\n", key, _item->m_int);
			break;
		case HistItem::TypeString:
        {
			fprintf(out, "%s = ", key);
            
            // Output the value, remembering to escape any backslash and newline characters
            char *c = _item->m_str;
            while (*c)
            {
                if (*c == '\n')
                    fprintf(out, "\\n");
                else if (*c == '\r')
                    fprintf(out, "\\r");
                else if (*c == '\\')
                    fprintf(out, "\\\\");
                else
                    fputc(*c, out);
                c++;
            }
            fputc('\n', out);
			break;
        }
	}
}


void WidgetHistory::Save()
{
	FILE *out = fopen(m_filename, "w");
	
	// If we couldn't open the prefs file for writing then just silently fail - 
	// it's better than crashing.
	if (!out)
		return;

	// Finally output any items that haven't already been written
	for (unsigned int i = 0; i < m_items.Size(); ++i)
	{
		if (m_items.ValidIndex(i)) 
		{
			HistItem *item = m_items.GetData(i);
			char const *key = m_items.GetName(i);
			SaveItem(out, key, item);
		}
	}

	fclose(out);
}


float WidgetHistory::GetFloat(char const *_key, float _default) const
{
	int index = m_items.GetIndex(_key);
	if (index == -1) return _default;
	HistItem *item = m_items.GetData(index);
	if (item->m_type != HistItem::TypeFloat) return _default;
	return item->m_float;
}


int WidgetHistory::GetInt(char const *_key, int _default) const
{
	int index = m_items.GetIndex(_key);
	if (index == -1) return _default;
	HistItem *item = m_items.GetData(index);
	if (item->m_type != HistItem::TypeInt) return _default;
	return item->m_int;
}


char *WidgetHistory::GetString(char const *_key, char *_default) const
{
	int index = m_items.GetIndex(_key);
	if (index == -1) return _default;
	HistItem *item = m_items.GetData(index);
	if (item->m_type != HistItem::TypeString) return _default;
	return item->m_str;
}


void WidgetHistory::SetString(char const *_key, char const *_string)
{
	int index = m_items.GetIndex(_key);

	if (index == -1)
	{
		HistItem *item = new HistItem(_string);
		m_items.PutData(_key, item);
	}
	else
	{
		HistItem *item = m_items.GetData(index);
        item->m_type = HistItem::TypeString;
		char *newString = StringDuplicate(_string);
        delete [] item->m_str;
		item->m_str = newString;
	}
}


void WidgetHistory::SetFloat(char const *_key, float _float)
{
	int index = m_items.GetIndex(_key);

	if (index == -1)
	{
		HistItem *item = new HistItem(_float);
		m_items.PutData(_key, item);
	}
	else
	{
		HistItem *item = m_items.GetData(index);
		item->m_type = HistItem::TypeFloat;
		item->m_float = _float;
	}
}


void WidgetHistory::SetInt(char const *_key, int _int)
{
	int index = m_items.GetIndex(_key);

	if (index == -1)
	{
		HistItem *item = new HistItem(_int);
		m_items.PutData(_key, item);
	}
	else
	{
		HistItem *item = m_items.GetData(index);
		item->m_type = HistItem::TypeInt;
		item->m_int = _int;
	}
}


bool WidgetHistory::DoesKeyExist(char const *_key)
{
	int index = m_items.GetIndex(_key);

	return index != -1;
}


char *WidgetHistory::ExecuteCommand(char const *object, char const *command, char const *arguments)
{ 
    if (COMMAND_IS("GetInt"))
    {
        int val = GetInt(arguments, 0);
        return CreateIntToSend(val);
    }
    
    return COMMAND_RETURN_NOTHING;
}
