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


bool BinaryFileWriter::WriteBytes(char const *buf, int64_t count)
{
    return fwrite(buf, 1, count, m_file) == count;
}


// ***************************************************************************
// BinaryDataWriter
// ***************************************************************************


BinaryDataWriter::BinaryDataWriter(int64_t numBytes)
{
	m_dataLen = numBytes;
	m_data = new uint8_t[m_dataLen];
	m_pos = 0;
}


void BinaryDataWriter::Reserve(int64_t numBytes)
{
	if (numBytes > m_dataLen)
	{
		uint8_t *newData = new uint8_t[numBytes];
		memcpy(newData, m_data, m_dataLen);
		m_dataLen = numBytes;
		delete[] m_data;
		m_data = newData;
	}
}


bool BinaryDataWriter::WriteU8(uint8_t val)
{
	if (m_pos < m_dataLen)
	{
		m_data[m_pos] = val;
		m_pos++;
		return true;
	}

	return false;
}


bool BinaryDataWriter::WriteU16(uint16_t val)
{
	if (m_pos + 1 < m_dataLen)
	{
		uint16_t *data = (uint16_t*)(m_data + m_pos);
		*data = val;
		m_pos += 2;
		return true;
	}

	return false;
}


bool BinaryDataWriter::WriteU32(uint32_t val)
{
	if (m_pos + 3 < m_dataLen)
	{
		uint32_t *data = (uint32_t*)(m_data + m_pos);
		*data = val;
		m_pos += 4;
		return true;
	}

	return false;
}


bool BinaryDataWriter::WriteBytes(char const *buf, int64_t count)
{
	if (m_pos + count <= m_dataLen)
	{
		memcpy(m_data + m_pos, buf, count);
		m_pos += count;
		return true;
	}

	return false;
}
