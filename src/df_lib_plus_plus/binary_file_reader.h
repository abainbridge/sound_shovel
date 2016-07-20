#pragma once


#include <stdio.h>
#include <stdint.h>


class BinaryFileReader
{
public:
    FILE *m_file;

    BinaryFileReader(char const *filename);
    ~BinaryFileReader();

    uint8_t ReadU8();
    uint16_t ReadU16();
    uint32_t ReadU32();

    unsigned ReadBytes(unsigned count, unsigned char *buf);
};


class BinaryFileWriter
{
public:
    FILE *m_file;

    BinaryFileWriter(char const *filename);
    ~BinaryFileWriter();

    bool WriteU8(uint8_t val);
    bool WriteU16(uint16_t val);
    bool WriteU32(uint32_t val);

    bool WriteBytes(char const *buf, unsigned count);
    bool WriteUBytes(unsigned char const *buf, unsigned count);
};
