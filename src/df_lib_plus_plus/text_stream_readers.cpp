// Own header
#include "text_stream_readers.h"

// Project headers
#include "andy_string.h"

// Contrib headers
#include "df_common.h"

// Platform headers
#ifdef WIN32
#include <io.h>
#endif

// Standard headers
#include <stdio.h>
#include <string.h>


#define DEFAULT_SEPERATOR_CHARS " \t\n\r:,"
#define INITIAL_LINE_LEN 128


//*****************************************************************************
// Class TextReader
//*****************************************************************************

TextReader::TextReader()
:	m_maxLineLen(INITIAL_LINE_LEN),
	m_lineNum(0),
	m_offsetIndex(0)
{
	m_filename[0] = '\0';
	m_line = new char[INITIAL_LINE_LEN + 1];	// Don't forget space for the null terminator
	strcpy(m_seperatorChars, DEFAULT_SEPERATOR_CHARS);
}


TextReader::~TextReader()
{
	delete [] m_line;
}


void TextReader::DoubleMaxLineLen()
{
	ReleaseAssert(m_maxLineLen < 65536, "Text file contains line with more than 65536 characters");
	char *newLine = new char [m_maxLineLen * 2 + 1];
	memcpy(newLine, m_line, m_maxLineLen + 1);
	delete m_line;
	m_line = newLine;
	m_maxLineLen *= 2;
}


void TextReader::CleanLine()
{
	// Scan for comments (which we remove) and merge conflict markers (which we assert on)
	
	int numAdjacentAngleBracketsFound = 0;
	char *c = m_line;
	while (c[0] != '\0')
	{
		if (c[0] == '<' || c[0] == '>')
		{
			numAdjacentAngleBracketsFound++;
			DebugAssert(numAdjacentAngleBracketsFound < 3);
            // Looks like you've got a merge error mate
		}
		else
		{
			numAdjacentAngleBracketsFound = 0;
			
			// Convert comment chars into newlines (unless they are escaped)
			if (c[0] == '#')
			{
				if (c > m_line && c[-1] == '\\')
				{
					// # is escaped. Remove the escape char
					c[-1] = ' ';
				}
				else
				{
					c[0] = '\n';
				}
			}
			
			// Strip the newline character
			if (c[0] == '\n')
			{
				c[0] = '\0';
				break;
			}
		}
		c++;
	}
}


void TextReader::SetSeperatorChars(char const *_seperatorChars)
{
	strncpy(m_seperatorChars, _seperatorChars, 15);
}


void TextReader::SetDefaultSeperatorChars()
{
	strcpy(m_seperatorChars, DEFAULT_SEPERATOR_CHARS);
}


bool TextReader::TokenAvailable()
{
	unsigned int i = m_tokenIndex;
	
	while (m_line[i] != '\0')
	{
		if (strchr(m_seperatorChars, m_line[i]) == NULL)
		{
			return true;
		}

		++i;
	}

	return false;
}


char const *TextReader::GetFilename()
{
	return m_filename;
}


// Returns the next token on the current line. Strips all separator
// characters from the start and end of the token, so "blah, wibble:7"
// would yield the tokens "blah", "wibble" and "7". Trailing separator 
// characters at the end of the line, then they are discarded. Double
// quotes (") can be used to create tokens containing whitespace. 
char *TextReader::GetNextToken()
{
	// Make sure there is more input on the line
	if (m_line[m_tokenIndex] == '\0')
	{
		return NULL;
	}

	// Skip over initial separator characters
	int m_tokenStart = m_tokenIndex;
	while(m_line[m_tokenStart] != '\0' &&
		  strchr(m_seperatorChars, m_line[m_tokenStart]) != NULL)
	{
		m_tokenStart++;
	}

	// Make sure that we haven't found an empty token
	if (m_line[m_tokenStart] == '\0')
	{
		DebugAssert(false);
		return NULL;
	}

	// Find the end of the token
	m_tokenIndex = m_tokenStart;
	bool inQuotes = false;
	while(m_line[m_tokenIndex] != '\0')
	{
		if (strchr(m_seperatorChars, m_line[m_tokenIndex]) != NULL)
		{
			if (!inQuotes)
			{
				m_line[m_tokenIndex] = '\0';
				m_tokenIndex++;
				break;
			}
		}
		else if (m_line[m_tokenIndex] == '"' && m_line[m_tokenIndex-1] != '\\')
		{
			if (inQuotes)
			{
				m_line[m_tokenIndex] = '"';
				m_tokenIndex++;
				m_line[m_tokenIndex] = '\0';
				m_tokenIndex++;
				break;
			}
			else
			{
				inQuotes = true;
			}
		}
		m_tokenIndex++;
	}

	return &m_line[m_tokenStart];
}


char *TextReader::GetRestOfLine()
{
	// Make sure there is more input on the line
	if (m_line[m_tokenIndex] == '\0')
	{
		return NULL;
	}

	// Skip over initial separator characters
	int m_tokenStart = m_tokenIndex;
	while(m_line[m_tokenStart] != '\0' &&
		  strchr(m_seperatorChars, m_line[m_tokenStart]) != NULL)
	{
		m_tokenStart++;
	}

    return &m_line[m_tokenStart];
}



//*****************************************************************************
// Class TextFileReader
//*****************************************************************************

TextFileReader::TextFileReader(char const *_filename)
:	TextReader()
{
	strncpy(m_filename, _filename, sizeof(m_filename) - 1);
	m_file = fopen(_filename, "r");
}


TextFileReader::~TextFileReader()
{
	if( m_file ) fclose(m_file);
}


bool TextFileReader::IsOpen()
{
    return( m_file && m_line );
}


// Reads a line from m_file. Removes comments that are marked with
// a hash ('#', aka the pound sign or indeed the octothorpe) character. 
// Returns 0 on EOF, 1 otherwise
bool TextFileReader::ReadLine()
{
	bool eof = false;
	m_tokenIndex = 0;

	
	//
	// Read some data from the file

	// If fgets returns NULL that means we've found the EOF
	if (fgets(m_line, m_maxLineLen + 1, m_file) == NULL)
	{
		m_line[0] = '\0';
		eof = true;
	}

	
	//
	// Make sure we read a whole line

	bool found = false;
	while (!found && !eof)
	{
		if (strchr(m_line, '\n'))
		{
			found = true;
		}

		if (!found)
		{
			unsigned int oldLineLen = m_maxLineLen;
			DoubleMaxLineLen();
			if (fgets(&m_line[oldLineLen], oldLineLen + 1, m_file) == NULL)
			{
				eof = true;
			}
		}
	}

	CleanLine();	
	m_lineNum++;

	return !eof;
}
