// Own header
#include "filesys_utils.h"

// Project headers
#include "andy_string.h"
#include "string_utils.h"

// Platform headers
#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

// Standard headers
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>


// ***************************************************************************
// Class FileDetails
// ***************************************************************************

FileDetails::FileDetails(char const *fullPath)
{
	SetFullPath(fullPath);
}


void FileDetails::SetFullPath(char const *fullPath)
{
	DebugAssert(fullPath[0]);

	if (IsRelativePath(fullPath))
	{
		char buf[MAX_PATH + 1];
		GetCwd(buf, MAX_PATH);
		m_fullPath = buf;
		m_fullPath += '/';
		m_fullPath += fullPath;
	}
	else
	{
		m_fullPath = fullPath;
	}

	CanonicalisePath(m_fullPath);

	// m_directoryPart
	int lastSlashIndex = m_fullPath.rfind('/');
	if (lastSlashIndex < 0)
	{
		m_directoryPart = m_fullPath;
	}
	else
	{
		lastSlashIndex++;
		m_directoryPart = m_fullPath.substr(0, lastSlashIndex);

		// m_filenamePart
		m_filenamePart = m_fullPath.substr(lastSlashIndex);
	}

	// m_extensionPart
	int lastDotIndex = m_fullPath.rfind('.');
	if (lastDotIndex != -1)
	{
		m_extensionPart = m_fullPath.substr(lastDotIndex + 1);
	}
}


void FileDetails::SetInvalid(char const *invalidPath)
{
	m_fullPath = invalidPath;
	m_filenamePart = "";
	m_directoryPart = "";
	m_extensionPart = "";
}


bool FileDetails::Exists()
{
	if (IsInvalid()) return false;
	return FileExists(m_fullPath.c_str());
}


bool FileDetails::IsReadOnly()
{
	if (IsInvalid()) return false;
    return ::IsReadOnly(m_fullPath.c_str());
}


int64_t FileDetails::GetSize()
{
	if (IsInvalid()) return 0;
    return GetFileSize(m_fullPath.c_str());
}


unsigned FileDetails::GetModificationTime()
{
	struct stat fileStat;
	if (IsInvalid()) return 0;
	int err = stat(m_fullPath.c_str(), &fileStat);
	if (err != 0) return 0;
	return fileStat.st_mtime;
}


void FileDetails::FormatTime(time_t _time, char *buf)
{
	time_t clockNow = time(NULL);
	struct tm *thenStruct = localtime(&clockNow);
	struct tm nowStruct;
	memcpy(&nowStruct, thenStruct, sizeof(struct tm));
	thenStruct = localtime(&_time);
	
	int daysAgo = nowStruct.tm_yday - thenStruct->tm_yday +
					365 * (nowStruct.tm_year - thenStruct->tm_year);

	if (daysAgo == 0)		sprintf(buf, "Today %02d:%02d", thenStruct->tm_hour, thenStruct->tm_min);
	else if (daysAgo == 1)	sprintf(buf, "Yesterday %02d:%02d", thenStruct->tm_hour, thenStruct->tm_min);
	else if (daysAgo < 10)	sprintf(buf, "%d days ago", daysAgo);
	else if (nowStruct.tm_year == thenStruct->tm_year)
	{
		static char *monthNames[12] = {
			"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
		};
		sprintf(buf, "%d %s", thenStruct->tm_mday, monthNames[thenStruct->tm_mon]);
	}
	else
	{
		sprintf(buf, "%d/%d/%d", 
			thenStruct->tm_year + 1900, thenStruct->tm_mon + 1, thenStruct->tm_mday);
	}	
}


bool FileDetails::operator == (FileDetails const &b)
{
	return stricmp(m_fullPath.c_str(), b.m_fullPath.c_str()) == 0;
}



//*****************************************************************************
// Class DirectoryList
//*****************************************************************************

DirectoryList::DirectoryList(char const *fullPath)
{
	m_error = NULL;

	// Copy the full path passed in
	int len = strlen(fullPath);
	m_fullPath = new char [len + 3];
	strcpy(m_fullPath, fullPath);
	strcat(m_fullPath, "/*");

	// Convert to Unix style directory separators
	int i = 0;
	while (m_fullPath[i])
	{
		if (m_fullPath[i] == '\\') m_fullPath[i] = '/';
		i++;
	}

	// Talk to the filing system
	WIN32_FIND_DATA item;
	HANDLE dirHandle = FindFirstFile(m_fullPath, &item);
	m_fullPath[len] = '\0';
	if (dirHandle == INVALID_HANDLE_VALUE)
	{
		int errorCode = GetLastError();
		switch (errorCode)
		{
		case 3:
			m_error = StringDuplicate("Path not found");
			break;
		default:
			m_error = new char [512];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
				errorCode, 0, m_error, 512, NULL);
			break;
		}
	}
	else
	{
		while (dirHandle) 
		{
			if (strcmp(item.cFileName, ".") != 0 &&
				strcmp(item.cFileName, "..") != 0)
			{
				DirectoryItem *ourItem = new DirectoryItem;
				ourItem->m_isDirectory = (item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
				ourItem->m_name = StringDuplicate(item.cFileName);
				m_items.PutData(ourItem);
			}
			if (!FindNextFile(dirHandle, &item))
			{
				break;
			}
		}

		unsigned dwError = GetLastError();
		if (dwError == ERROR_NO_MORE_FILES)
			FindClose(dirHandle);
	}
}


DirectoryList::~DirectoryList()
{
	delete [] m_fullPath;
	m_items.EmptyAndDelete();
}


//*****************************************************************************
// Misc directory and filename functions
//*****************************************************************************

bool AreFilesIdentical(char const *_name1, char const *_name2)
{
	FILE *in1 = fopen(_name1, "r");
	FILE *in2 = fopen(_name2, "r");
	bool rv = true;
	bool exitNow = false;

	if (!in1 || !in2)
	{
		rv = false;
		exitNow = true;
	}

	while (exitNow == false && !feof(in1) && !feof(in2))
	{
		int a = fgetc(in1);
		int b = fgetc(in2);
		if (a != b) 
		{
			rv = false;
			exitNow = true;
			break;
		}
	}

	if (exitNow == false && feof(in1) != feof(in2))
	{
		rv = false;
	}

	if (in1) fclose(in1);
	if (in2) fclose(in2);

	return rv;
}


bool IsDirectory(char const *fullPath)
{
	unsigned int attribs = GetFileAttributes(fullPath);
	return attribs & FILE_ATTRIBUTE_DIRECTORY;
}


bool IsReadOnly(char const *fullPath)
{
    unsigned int attribs = GetFileAttributes(fullPath);
    if (attribs == 0xFFFFFFFF)
        return false;
    return attribs & FILE_ATTRIBUTE_READONLY;
}


bool MakeWritable(char const *fullPath)
{
    unsigned int attribs = GetFileAttributes(fullPath);
    if (attribs == 0xFFFFFFFF)
        return false;
    attribs &= ~FILE_ATTRIBUTE_READONLY;
    return SetFileAttributes(fullPath, attribs);
}


bool FileExists(char const *fullPath)
{
	unsigned int attribs = GetFileAttributes(fullPath);
	return attribs != -1;
}


unsigned GetFileSize(char const *fullPath)
{
    struct _stati64 fileStat;
    int err = _stati64(fullPath, &fileStat);
    if (err != 0) return 0;
    return fileStat.st_size;
}


bool CreateDirectory(char const *_directory)
{
#ifdef _WIN32
	if (_mkdir(_directory) != -1) 
		return true;

	String reducedDir = _directory;
	int index = reducedDir.rfind('/');
	if (index == String::npos)
		index = reducedDir.rfind('\\');
	if (index != String::npos && index > 2)
	{
		reducedDir.erase(index);
		bool boolResult = CreateDirectory(reducedDir.c_str());
		if (boolResult == true)
		{
			int result = _mkdir(_directory);
			if (result == 0) return true;
		}
	}

	return false;
#else
    ReleaseAssert( false, "CreateDirectory not yet implemented" );
#endif
}


bool RemoveFile(char const *_filename)
{
	return DeleteFile(_filename);
}


bool MoveFile_(char const *_fullPathSrc, char const *_fullPathDst)
{
    if (FileExists(_fullPathDst))
        RemoveFile(_fullPathDst);
	int i = rename(_fullPathSrc, _fullPathDst);
	return i == 0;
}


void GetCwd(char *buffer, int max_len)
{
	getcwd(buffer, max_len);
}


void SetCwd(char const *dir)
{
	chdir(dir);
}


static void CorrectPathCase(String &path)
{
	// Make sure path exists
	if (!FileExists(path.c_str()))
		return;

	// If this path starts with a drive letter, then make it uppercase.
	// If not then it's a '/' so toupper will have no effect.
	path[0] = toupper(path[0]);


	//
	// Now go through the path one "part" at a time.  A part is either 
	// directory name, or the filename. We are going to "look up" the correct
	// case for each part and modify it in place
	
	// Start after the first slash
	int i = path.find('/', 0);
    if (i > -1)
    {
        i++;
	    int slashPos;
	    while (1)
	    {
		    slashPos = path.find('/', i);
		    if (slashPos == -1)
			    slashPos = path.length();
		    else
			    path[slashPos] = '\0';

		    // Ask the file system to find something with this name
		    WIN32_FIND_DATA item;
		    HANDLE fh = FindFirstFile(path.c_str(), &item);
		    
		    // fh should only ever be NULL when FileExists is no longer true (very 
		    // unlikely since we checked for that above).
		    if (!fh) break;

		    FindClose(fh);

		    // Replace our version of the part's name with the one the file system returned
		    int partLength = slashPos - i;
		    path.replace(i, partLength, item.cFileName);

    	    i += strlen(item.cFileName);

		    // Have we reached the end of the path?
		    if (i >= path.length())
			    break;

		    // Get ready for next iteration
		    path[i] = '/';	
		    i++;
	    }
    }
}


// Converts any '\' into '/'
// Replaces 'foo//bar' with 'foo/bar'
// Strips trailing slashes
void CanonicalisePath(String &path)
{
    if (path.length() == 0)
        return;

	for (int i = 0; i < path.length(); ++i)
	{
		if (path[i] == '\\')
		{
			path[i] = '/';
		}
	}

	// Reduce multiple adjacent slashes to singles
	int i = path.find("//");
	while (i != String::npos)
	{
		path.erase(i, 1);
		i = path.find("//");
	}

	// Strip trailing slash
	if (path[path.length() - 1] == '/')
	{
		path.erase(path.length() - 1);
	}

    // Remove ".."s
    int dotIndex = -1;
    while ((dotIndex = path.find("..")) != -1)
    {
        int slashPos = path.rfind('/', dotIndex - 2);
        path.erase(slashPos, dotIndex - slashPos + 2);
    }

	CorrectPathCase(path);

	// Restore double slash at the beginning of UNC paths
	if (path[0] == '/')
	{
		path = '/' + path;
	}
}


void WindowsFilePath(char *path)
{
	int i = 0;
	while (path[i])
	{
		if (path[i] == '/') path[i] = '\\';
		i++;
	}
}


bool IsRelativePath(char const *path)
{
	int len = strlen(path);
	if (len < 3) return true;
	if (path[1] == ':') return false;
	if (path[0] == '/' && path[1] == '/') return false;
	if (path[0] == '\\' && path[1] == '\\') return false;
	return true;
}


char const *GetExePath()
{
    HMODULE mod = GetModuleHandle(NULL);
    static char buf[MAX_PATH + 1];
    GetModuleFileName(mod, buf, MAX_PATH + 1);
    return buf;
}
