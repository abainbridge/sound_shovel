// Own header
#include "binary_file_reader.h"

// Platform headers
#ifdef WIN32
#include <io.h>
#endif

// Standard headers
#include <stdio.h>
#include <string.h>


BinaryFileReader::BinaryFileReader(char const *filename)
{
    strncpy(m_filename, filename, sizeof(m_filename) - 1);
    m_file = fopen(filename, "rb");
}


BinaryFileReader::~BinaryFileReader()
{
	if (m_file)
		fclose(m_file);
}


bool BinaryFileReader::IsOpen()
{
	return !!m_file;
}


bool BinaryFileReader::IsEof()
{
    return !!feof(m_file);
}


int8_t BinaryFileReader::ReadS8()
{
	return fgetc(m_file);
}


int16_t BinaryFileReader::ReadS16()
{
    int16_t val;
    fread((void*)&val, 2, 1, m_file);
	return val;
}


int32_t BinaryFileReader::ReadS32()
{
    int32_t val;
    fread((void*)&val, 4, 1, m_file);
	return val;
}


uint8_t BinaryFileReader::ReadU8()
{
    return fgetc(m_file);
}


uint16_t BinaryFileReader::ReadU16()
{
    uint16_t val;
    fread((void*)&val, 2, 1, m_file);
    return val;
}


uint32_t BinaryFileReader::ReadU32()
{
    uint32_t val;
    fread((void*)&val, 4, 1, m_file);
    return val;
}


unsigned BinaryFileReader::ReadBytes(unsigned count, unsigned char *buffer)
{
	return fread(buffer, 1, count, m_file);
}


int BinaryFileReader::Seek(int offset, int origin)
{
	return fseek(m_file, offset, origin);
}


int BinaryFileReader::Tell()
{
	return ftell(m_file);
}
