#pragma once
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
inline bool IsSpace(int c)
{
	return !!isspace(c & 0xff);
}


inline bool IsDigit(int c)
{
	return !!isdigit(c & 0xff);
}


inline bool IsWordChar(int c)
{
	return !!isalnum(c & 0xff) || c == '_';
}
