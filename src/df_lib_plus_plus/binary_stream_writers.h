#pragma once


#include <stdio.h>
#include <stdint.h>


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
