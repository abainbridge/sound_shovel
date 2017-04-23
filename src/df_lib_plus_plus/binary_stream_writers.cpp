// Own header
#include "binary_stream_writers.h"

// Platform headers
#ifdef WIN32
//#include <io.h>
#endif

// Standard headers
#include <stdio.h>
#include <string.h>


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
