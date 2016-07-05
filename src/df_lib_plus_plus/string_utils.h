#ifndef INCLUDED_STRING_UTILS_H
#define INCLUDED_STRING_UTILS_H

#include <ctype.h>


bool StringStartsWith(char const *theString, char const *prefix);
char *StringDuplicate(char const *theString);
void StringToLower(char *theString);
char *EscapeSpecialReChars(char const *searchString);
void ConvertGlobIntoRegExp(char const *glob, char *reBuf, int reBufLen);
void MakeNiceNumberString(char *buf, int num);
bool IsWordBreak(char a, char const *l, int x);


// isspace fails an assert if c is a high-ascii character and char is signed.
// This function doesn't.
inline bool safe_isspace(int c)
{
	return !!isspace(c & 0xff);
}


inline bool safe_isdigit(int c)
{
	return !!isdigit(c & 0xff);
}


inline bool iswordchar(int c)
{
	return !!isalnum(c & 0xff) || c == '_';
}


#endif
