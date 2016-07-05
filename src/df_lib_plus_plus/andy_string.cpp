// Own header
#include "andy_string.h"

// Standard headers
#include <string.h>


#ifdef noinl

const unsigned String::npos = -1;


// ***************************************************************************
// Private Functions
// ***************************************************************************

// For some bizarre reason, this strlen generates better code in VS 2008 (x86)
// than the one in the standard library. This code compiles down to 3
// instructions in the loop, compared to the standard implementation with 4.
// Also, when inlined it tends to save another instruction or two compared to
// the standard one.
inline static unsigned mystrlen(char const *s)
{
    unsigned lenS = 0;
    while (s[lenS])
        lenS++;
    return lenS;
}


void String::assign(char const *s)
{
    unsigned lenS = strlen(s);
    assign(s, lenS);
}


void String::assign(char const *s, unsigned len)
{
    m_array = new char [len + 1];
    for (unsigned i = 0; i < len; i++)
        m_array[i] = s[i];
    m_array[len] = '\0';
    m_len = len;
}


void String::join(char const *lhs, unsigned lenLhs, char const *rhs, unsigned lenRhs)
{
    m_len = lenLhs + lenRhs;
    char *newArray = new char [m_len + 1];
    for (unsigned i = 0; i < lenLhs; i++)
        newArray[i] = lhs[i];
    for (unsigned i = 0; i < lenRhs; i++)
        (newArray + lenLhs)[i] = rhs[i];
    newArray[m_len] = '\0';
    delete [] m_array;
    m_array = newArray;
}


// ***************************************************************************
// Public Functions
// ***************************************************************************

String::String()
{
    assign("", 0);
}


String::String(char const *s)
{
    assign(s);
}


String::String(const String &s)
{
    assign(s.m_array, s.m_len);
}


String::~String()
{
    delete [] m_array;
}


void String::resize(size_t n, char c)
{
    if (n == m_len)
        return;

    char *newArray = new char [n + 1];

    if (n < m_len)
    {
        memcpy(newArray, m_array, n);
    }
    else
    {
        memcpy(newArray, m_array, m_len);
        memset(newArray + m_len, c, n - m_len);
    }
    newArray[n] = '\0';

    delete [] m_array;
    m_array = newArray;
    m_len = n;
}


void String::clear()
{
    delete [] m_array;
    assign("", 0);
}


void String::operator = (String const &s)
{
    delete [] m_array;
    assign(s.m_array, s.m_len);
}


void String::operator = (char const *s)
{
    delete [] m_array;
    assign(s);
}


void String::operator += (String const &s)
{
    join(m_array, m_len, s.m_array, s.m_len);
}


void String::operator += (char const *s)
{
    unsigned sLen = mystrlen(s);
    join(m_array, m_len, s, sLen);
}


void String::operator += (char c)
{
    push_back(c);
}


void String::append(const char* s, size_t n)
{
    join(m_array, m_len, s, n);
}


void String::append(const String &s, size_t subpos, size_t sublen)
{
    join(m_array, m_len, s.m_array + subpos, sublen);
}


void String::push_back(char c)
{
    char s[2];
    s[0] = c;
    s[1] = '\0';
    join(m_array, m_len, s, 1);
}


void String::insert(size_t pos, const char *s)
{
    unsigned lenS = mystrlen(s);
    unsigned newMLen = m_len + lenS;
    char *newArray = new char [newMLen + 1];
    for (unsigned i = 0; i < pos; i++)
        newArray[i] = m_array[i];
    for (unsigned i = 0; i < lenS; i++)
        newArray[pos + i] = s[i];
    for (unsigned i = 0; i < m_len - pos; i++)
        newArray[pos + lenS + i] = m_array[pos + i];
    newArray[newMLen] = '\0';
    delete [] m_array;
    m_array = newArray;
    m_len = newMLen;
}


void String::insert(size_t pos, size_t n, char c)
{
    unsigned newMLen = m_len + n;
    char *newArray = new char [newMLen + 1];
    for (unsigned i = 0; i < pos; i++)
        newArray[i] = m_array[i];
    for (unsigned i = 0; i < n; i++)
        newArray[pos + i] = c;
    for (unsigned i = 0; i < m_len - pos; i++)
        newArray[pos + n + i] = m_array[pos + i];
    newArray[newMLen] = '\0';
    delete [] m_array;
    m_array = newArray;
    m_len = newMLen;
}


void String::erase(size_t pos, size_t len)
{
    if (pos >= m_len)
        return;

    if (len == npos || (pos + len) > m_len)
        len = m_len - pos;

    join(m_array, pos, m_array + pos + len, m_len - pos - len);
}


void String::replace(size_t pos, size_t len, char const *s)
{
	if (pos + len > m_len)
		return;
    unsigned lenS = mystrlen(s);
    unsigned newMLen = m_len - len + lenS;
    char *newArray = new char [newMLen + 1];
    for (unsigned i = 0; i < pos; i++)
        newArray[i] = m_array[i];
    for (unsigned i = 0; i < lenS; i++)
        newArray[pos + i] = s[i];
    for (unsigned i = 0; i < m_len - (pos + len); i++)
        newArray[pos + lenS + i] = m_array[pos + len + i];
    newArray[newMLen] = '\0';
    delete [] m_array;
    m_array = newArray;
    m_len = newMLen;
}


size_t String::find(char const *s, size_t pos) const
{
    unsigned lenS = mystrlen(s);
    for (unsigned i = pos; i <= m_len - lenS; i++)
        if (strncmp(m_array + i, s, lenS) == 0)
            return i;
    return npos;
}


size_t String::find(char c, size_t pos) const
{
    for (unsigned i = pos; i < m_len; i++)
        if (m_array[i] == c)
            return i;

    return npos;
}


size_t String::rfind(char c, size_t pos) const
{
    for (unsigned i = m_len - 1; i; i--)
        if (m_array[i] == c)
            return i;
    return npos;
}


size_t String::find_first_of(const char *s, size_t pos) const
{
    unsigned lenS = mystrlen(s);
    for (unsigned i = pos; i < m_len; i++)
        for (unsigned j = 0; j < lenS; j++)
            if (m_array[i] == s[j])
                return i;

    return npos;
}


String String::substr(size_t pos, size_t len) const
{
    if (len == npos)
        len = m_len - pos;
    String rv;
    rv.assign(m_array + pos, len);
    return rv;
}


String operator + (const String &lhs, const String &rhs)
{
    String rv(lhs);
    rv += rhs;
    return rv;
}


String operator + (const String &lhs, const char *rhs)
{
    String rv(lhs);
    rv += rhs;
    return rv;
}


String operator + (const char *lhs, const String &rhs)
{
    String rv(lhs);
    rv += rhs;
    return rv;
}


String operator + (const String &lhs, char rhs)
{
    String rv(lhs);
    rv += rhs;
    return rv;
}


String operator + (char lhs, const String &rhs)
{
    return operator + (rhs, lhs);
}


bool operator == (const String &lhs, const String &rhs)
{
    return operator == (lhs, rhs.m_array);
}


bool operator == (const String &lhs, const char *rhs)
{
    return strcmp(lhs.m_array, rhs) == 0;
}


bool operator != (const String &lhs, const char *rhs)
{
    return strcmp(lhs.m_array, rhs) != 0;
}


#endif
