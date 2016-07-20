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
    m_file = fopen(filename, "rb");
}


BinaryFileReader::~BinaryFileReader()
{
    if (m_file)
        fclose(m_file);
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



// ***************************************************************************
// BinaryFileWriter
// ***************************************************************************

BinaryFileWriter::BinaryFileWriter(char const *filename)
{
    m_file = fopen(filename, "wb");
}


BinaryFileWriter::~BinaryFileWriter()
{
    if (m_file)
        fclose(m_file);
}


bool BinaryFileWriter::WriteU8(uint8_t val)
{
    return fputc(val, m_file) != EOF;
}


bool BinaryFileWriter::WriteU16(uint16_t val)
{
    return fwrite((void*)&val, 2, 1, m_file) == 1;
}


bool BinaryFileWriter::WriteU32(uint32_t val)
{
    return fwrite((void*)&val, 4, 1, m_file) == 1;
}


bool BinaryFileWriter::WriteBytes(char const *buf, unsigned count)
{
    return fwrite(buf, 1, count, m_file) == count;
}


bool BinaryFileWriter::WriteUBytes(unsigned char const *buf, unsigned count)
{
    return fwrite(buf, 1, count, m_file) == count;
}
