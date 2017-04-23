#pragma once


#include <stdint.h>
#include <stdio.h>


//*****************************************************************************

class BinaryStreamReader
{
public:
	bool			m_eof;
	char			m_filename[256];

	BinaryStreamReader		 ();
	virtual ~BinaryStreamReader();

    virtual bool			IsOpen  () = 0;
	virtual char *			GetFileType() = 0;

	virtual uint8_t ReadU8() = 0;
	virtual uint16_t ReadU16() = 0;
	virtual uint32_t ReadU32() = 0;

	virtual unsigned int	ReadBytes(unsigned int _count, unsigned char *_buffer) = 0;

	virtual int				Seek	(int _offset, int _origin) = 0;
	virtual int				Tell	() = 0;
};


//*****************************************************************************

class BinaryFileReader: public BinaryStreamReader
{
protected:
	FILE			*m_file;

public:
	BinaryFileReader			(char const *_filename);
	~BinaryFileReader			();

    bool			IsOpen		();
	char *			GetFileType	();

	uint8_t	ReadU8		();
	uint16_t ReadU16		();
	uint32_t ReadU32		();

	unsigned int	ReadBytes	(unsigned int _count, unsigned char *_buffer);

	int				Seek		(int _offset, int _origin);
	int				Tell		();
};


//*****************************************************************************

class BinaryDataReader: public BinaryStreamReader
{
protected:
	unsigned int		m_offset;

public:
	unsigned char const *m_data;
	unsigned int		m_dataSize;

	BinaryDataReader			(unsigned char const *_data, unsigned int _dataSize, 
								 char const *_filename);
	~BinaryDataReader			();

    bool			IsOpen		();
	char *			GetFileType	();

	uint8_t		ReadU8		();
	uint16_t			ReadU16		();
	uint32_t				ReadU32		();

	unsigned int	ReadBytes	(unsigned int _count, unsigned char *_buffer);

	int				Seek		(int _offset, int _origin);
	int				Tell		();
};
