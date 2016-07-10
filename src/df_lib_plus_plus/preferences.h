#pragma once


// Project headers
#include "containers/hash_table.h"
#include "containers/llist.h"

// Standard headers
#include <stdio.h>


// ***************
// Class PrefsItem
// ***************

class PrefsItem
{
public:
    enum ItemType
    {
        TypeString,
        TypeFloat,
        TypeInt,
        TypeUnknown
    };

    ItemType    m_type;
    bool        m_hasBeenWritten;           // Used when saving the prefs file
    bool        m_registered;
    bool        m_variesWithSyntaxGroup;

public:
    PrefsItem();
    virtual void WriteValIntoBuf(char *buf, unsigned bufSize) = 0;
    virtual bool SetValueFromLine(char const *line) = 0;    // Returns false if line specified an illegal value
};


class PrefsItemString: public PrefsItem
{
public:
    char        *m_str;
    PrefsItemString(char const *_str = "null");
    ~PrefsItemString();
    virtual void WriteValIntoBuf(char *buf, unsigned bufSize);
    virtual bool SetValueFromLine(char const *line);
};


class PrefsItemInt: public PrefsItem
{
public:
    int     m_int;
    int     m_min;
    int     m_max;
    PrefsItemInt(int _int, int _min, int _max);
    virtual void WriteValIntoBuf(char *buf, unsigned bufSize);
    virtual bool SetValueFromLine(char const *line);
};


class PrefsItemFloat: public PrefsItem
{
public:
    float   m_float;
    PrefsItemFloat(float _float = 0.0f);
    void WriteValIntoBuf(char *buf, unsigned bufSize);
    virtual bool SetValueFromLine(char const *line);
};


// ******************
// Class PrefsManager
// ******************

class PrefsManager
{
protected:
    char            *m_filename;
    LList           <char *> m_fileText;
    unsigned        m_diskFileModTimeWhenRead;

    bool IsLineEmpty(char const *_line);
    void SaveItem   (FILE *out, char const *key, PrefsItem *_item);

    void AddLine    (char *line);

public:
    HashTable       <PrefsItem *> m_items;

    PrefsManager();

    void Load       (char const *filename);
    void Save       ();

    PrefsItem *GetItem(char const *key) const;

    char  *GetString(char const *key) const;
    float  GetFloat (char const *key) const;
    int    GetInt   (char const *key) const;

    void RegisterInt(char const *name, int _default, int _min, int _max, bool variesWithSyntaxGroup);
    void RegisterFloat(char const *name, float _default, bool variesWithSyntaxGroup);
    void RegisterString(char const *name, char const *_default, bool variesWithSyntaxGroup);

    void GetBaseName(char const *fullName, char *baseName);

    // Creates an item that overrides a base item (syntax file parser and local settings parse both use this)
    PrefsItem *CreateOverrideItem(char const *fullName);

    // Set functions update existing PrefsItems or create new ones if key doesn't yet exist
    void SetString  (char const *_key, char const *_string);
    void SetFloat   (char const *_key, float _val);
    void SetInt     (char const *_key, int _val);
};


extern PrefsManager *g_prefs;
