#pragma once

#include <stdio.h>


//*****************************************************************************
// Class TextReader
//*****************************************************************************

// This is an ABSTRACT BASE class. If you want to actually tokenise some text,
// I recommend either TextFileReader or TextDataReader.
class TextReader
{
protected:
	char			m_seperatorChars[16];
	char			m_filename[256];

	int				m_offsetIndex;

	void			DoubleMaxLineLen();
	void			CleanLine();		// Decrypt, strip comments and scan for conflict markers

public:
	int				m_tokenIndex;
	char			*m_line;
	unsigned int	m_maxLineLen;		// Doesn't include '\0' - m_line points to an array one byte larger than this
	unsigned int	m_lineNum;

	TextReader		();
	virtual ~TextReader();

    virtual bool IsOpen         () = 0;
	virtual bool ReadLine		() = 0;	// Returns false on EOF, true otherwise
	bool		 TokenAvailable	();
	char		 *GetNextToken	();
    char		 *GetRestOfLine ();

	char const	*GetFilename	();

	void SetSeperatorChars		(char const *_seperatorChars);
	void SetDefaultSeperatorChars();
};


//*****************************************************************************
// Class TextFileReader
//*****************************************************************************

class TextFileReader: public TextReader
{
protected:
	FILE			*m_file;

public:
	TextFileReader				(char const *_filename);
	~TextFileReader				();

    bool IsOpen                 ();
	bool ReadLine				();
};
