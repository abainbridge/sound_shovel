#pragma once


#include <stdio.h>
#include <stdint.h>


class BinaryFileReader
{
private:
    FILE *m_file;

public:
    char m_filename[256];

    BinaryFileReader(char const *filename);
    ~BinaryFileReader();

    bool IsOpen();
    bool IsEof();

    int8_t ReadS8();
    int16_t ReadS16();
    int32_t ReadS32();

    uint8_t ReadU8();
    uint16_t ReadU16();
    uint32_t ReadU32();

    unsigned ReadBytes(unsigned count, unsigned char *buffer);

    int Seek(int offset, int origin);
    int Tell();
};
