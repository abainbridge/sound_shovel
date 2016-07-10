#pragma once


// Project headers
#include "containers/hash_table.h"
#include "command.h"

// Standard headers
#include <stdio.h>


// ***************
// Class HistItem
// ***************

class HistItem
{
public:
	enum
	{
		TypeString,
		TypeFloat,
		TypeInt
	};
	
	int			m_type;
	char		*m_str;
	union 
	{
		int		m_int;
		float	m_float;
	};

	HistItem(): m_str(NULL) {};
	HistItem(char *_line);
	HistItem(char const *_str);
	HistItem(float _float);
	HistItem(int _int);
	~HistItem();

	void SetValueFromLine(char const *line);
};


// *******************
// Class WidgetHistory
// *******************

class WidgetHistory: public CommandReceiver
{
private:
	HashTable       <HistItem *> m_items;
    char            *m_filename;
    unsigned        m_diskFileModTimeWhenRead;

	bool IsLineEmpty(char const *_line);
	void SaveItem	(FILE *out, char const *key, HistItem *_item);

	char *GetKeyFromLine(char *line);

public:
	WidgetHistory(char const *_filename);
	~WidgetHistory();

	void Load		(char const *_filename=NULL); // If filename is NULL, then m_filename is used
	void Save		();

	char *GetString (char const *_key, char *_default=NULL) const;
	float GetFloat  (char const *_key, float _default=-1.0f) const;
	int	  GetInt    (char const *_key, int _default=-1) const;

	// Set functions update existing HistItems or create new ones if key doesn't yet exist
	void SetString  (char const *_key, char const *_string);
	void SetFloat	(char const *_key, float _val);
	void SetInt		(char const *_key, int _val);
    
    void AddLine    (char *_line);

    bool DoesKeyExist(char const *_key);

    virtual char *ExecuteCommand(char const *object, char const *command, char const *arguments);
};


extern WidgetHistory *g_widgetHistory;
