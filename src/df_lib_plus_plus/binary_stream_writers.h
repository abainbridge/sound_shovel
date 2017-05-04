#pragma once


#include <stdio.h>
#include <stdint.h>


class BinaryStreamWriter
{
public:
	virtual void Reserve(int64_t numBytes) {}

    virtual bool WriteU8(uint8_t val) = 0;
    virtual bool WriteU16(uint16_t val) = 0;
    virtual bool WriteU32(uint32_t val) = 0;

    virtual bool WriteBytes(char const *buf, int64_t count) = 0;
};


class BinaryFileWriter: public BinaryStreamWriter
{
public:
    FILE *m_file;

    BinaryFileWriter(char const *filename);
    ~BinaryFileWriter();

	bool WriteU8(uint8_t val);
    bool WriteU16(uint16_t val);
    bool WriteU32(uint32_t val);

    bool WriteBytes(char const *buf, int64_t count);
};


class BinaryDataWriter: public BinaryStreamWriter
{
public:
    uint8_t *m_data;
    int64_t m_dataLen;
	int64_t m_pos;

	BinaryDataWriter(int64_t numBytes = 0);
	~BinaryDataWriter() { delete[] m_data; }

	void Reserve(int64_t numBytes);

	bool WriteU8(uint8_t val);
    bool WriteU16(uint16_t val);
    bool WriteU32(uint32_t val);

    bool WriteBytes(char const *buf, int64_t count);
};
