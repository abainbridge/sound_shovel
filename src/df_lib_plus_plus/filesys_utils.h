#pragma once


// Project headers
#include "andy_string.h"
#include "containers/llist.h"

// Standard headers
#include <stdint.h>
#include <time.h>


// ***************************************************************************
// Class FileDetails
// ***************************************************************************

class FileDetails
{
private:
	String	m_fullPath;			// eg c:/dir/another_dir/some_file.txt
	String	m_directoryPart;	// eg c:/dir/another_dir
	String	m_filenamePart;		// eg some_file.txt
	String	m_extensionPart;	// eg txt

public:
	FileDetails() {}
	FileDetails(char const *fullPath);

	void		SetFullPath			(char const *fullPath);		// Only valid paths allowed
	void		SetInvalid			(char const *invalidPath);

	char const *GetFullPath			() const { return m_fullPath.c_str(); };
	char const *GetDirectoryPart	() const { return m_directoryPart.c_str(); };
	char const *GetFilenamePart		() const { return m_filenamePart.c_str(); };
	char const *GetExtensionPart	() const { return m_extensionPart.c_str(); };

	bool		Exists				();	// Returns true if the file exists
	bool		IsReadOnly			();	// Returns true if the file is read only
	bool		IsInvalid			() { return m_directoryPart.length() == 0; }
	int64_t		GetSize				();	// Returns size of file as reported by the file system (in bytes)
	unsigned	GetModificationTime	(); // Returns the time the file was last modified, in seconds since God knows when
	void		FormatTime			(time_t _time, char *buf);	// Converts a timestamp from GetModificationTime in a human readable string time/date string. Buf must be at least 25 characters

	bool operator == (FileDetails const &);
};


//*****************************************************************************
// Class DirectoryList
//*****************************************************************************

class DirectoryItem
{
public:
	char *m_name;
	bool  m_isDirectory;

public:
	DirectoryItem(): m_name(NULL) {}
	~DirectoryItem() { delete [] m_name; }
};


class DirectoryList
{
public:
	char	*m_fullPath;
	LList	<DirectoryItem *> m_items;
	char	*m_error;

public:
	DirectoryList(char const *fullPath);
	~DirectoryList();
};


//*****************************************************************************
// Misc directory and filename functions
//*****************************************************************************

bool AreFilesIdentical	(char const *name1, char const *name2);
bool IsDirectory		(char const *fullPath);
bool IsReadOnly         (char const *fullPath);
bool MakeWritable       (char const *fullPath);
bool FileExists			(char const *fullPath);
unsigned GetFileSize    (char const *fullPath);
bool CreateDirectory    (char const *directory);
bool RemoveFile			(char const *filename);                                 // Returns true on success
bool MoveFile_			(char const *_fullPathSrc, char const *_fullPathDst);   // Returns true on success
void GetCwd				(char *buffer, int max_len);
void SetCwd				(char const *dir);
void CanonicalisePath	(String &path);
void WindowsFilePath	(char *path);
bool IsRelativePath		(char const *path);
char const *GetExePath  ();