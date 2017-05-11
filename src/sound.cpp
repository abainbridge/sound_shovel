#include "sound.h"

// Own header
#include "sound.h"

// Project headers
#include "sound_channel.h"
#include "df_lib_plus_plus/binary_stream_readers.h"
#include "df_lib_plus_plus/binary_stream_writers.h"
#include "df_lib_plus_plus/string_utils.h"

// Contrib headers
#include "df_time.h"

// Standard headers
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>


int const MAX_SAMPLE_VALUE = 32767;
int const MIN_SAMPLE_VALUE = -32768;


// ****************************************************************************
// Private Functions
// ****************************************************************************

void Sound::SetVolumeHelper(int64_t startIdx, int64_t endIdx, double startVol, double endVol)
{
    if (endIdx <= startIdx)
        return;

    int64_t len = endIdx - startIdx + 1;
    double volIncrement = (endVol - startVol) / (double)len;

    for (int j = 0; j < m_numChannels; j++)
    {
        SoundChannel *chan = m_channels[j];
        SoundChannel::SoundPos pos = chan->GetSoundPosFromSampleIdx(startIdx);
        SampleBlock *block = chan->m_blocks[pos.m_blockIdx];
        for (int64_t i = 0; i < len; i++)
        {
            double vol = startVol + (double)i * volIncrement;
            double newSampleValue = block->m_samples[pos.m_sampleIdx] * vol;
            ClampDouble(newSampleValue, MIN_SAMPLE_VALUE, MAX_SAMPLE_VALUE);
            block->m_samples[pos.m_sampleIdx] = newSampleValue;

            SampleBlock *nextBlock = chan->IncrementSoundPos(&pos, 1);
            if (nextBlock != block)
                block->RecalcLuts();
            if (!nextBlock)
                break;
            block = nextBlock;
        }

        block->RecalcLuts();
    }
}


// ****************************************************************************
// Public Functions
// ****************************************************************************

Sound::Sound()
{
    m_channels = NULL;
    m_numChannels = 0;
    m_cachedLength = -1;
    m_filename = "";
}


Sound::~Sound()
{
    for (int i = 0; i < m_numChannels; i++)
        delete m_channels[i];
    delete[] m_channels;
    delete[] m_filename;
}


void Sound::Delete(int64_t startIdx, int64_t endIdx)
{
    for (int i = 0; i < m_numChannels; i++)
        m_channels[i]->Delete(startIdx, endIdx);

    m_cachedLength = -1;
}


int Sound::Insert(int64_t startIdx, Sound *sound)
{
    if (sound->m_numChannels != m_numChannels)
        return ERROR_WRONG_NUMBER_OF_CHANNELS;
    
    for (int i = 0; i < m_numChannels; i++) {
        SoundChannel *srcChan = sound->m_channels[i];
        SoundChannel *dstChan = m_channels[i];
        dstChan->Insert(startIdx, srcChan);
    }

    m_cachedLength = -1;

    return ERROR_NO_ERROR;
}


void Sound::FadeIn(int64_t startIdx, int64_t endIdx)
{
    SetVolumeHelper(startIdx, endIdx, 0.0, 1.0);
}


void Sound::FadeOut(int64_t startIdx, int64_t endIdx)
{
    SetVolumeHelper(startIdx, endIdx, 1.0, 0.0);
}


void Sound::Normalize(int64_t startIdx, int64_t endIdx)
{
    if (endIdx <= startIdx)
        return;

    int64_t len = endIdx - startIdx + 1;
    int64_t maxAbsSample = 0;

    for (int j = 0; j < m_numChannels; j++)
    {
        SoundChannel *chan = m_channels[j];
        SoundChannel::SoundPos pos = chan->GetSoundPosFromSampleIdx(startIdx);
        SampleBlock *block = chan->m_blocks[pos.m_blockIdx];
        
        for (int64_t i = 0; i < len; i++)
        {
            int64_t sample = block->m_samples[pos.m_sampleIdx];
            sample = abs(sample);
            if (sample > maxAbsSample)
                maxAbsSample = sample;

            block = chan->IncrementSoundPos(&pos, 1);
            if (!block)
                break;
        }

        block->RecalcLuts();
    }

    double volChange = (double)MAX_SAMPLE_VALUE / (double)maxAbsSample;
    SetVolumeHelper(startIdx, endIdx, volChange, volChange);
}


bool Sound::LoadWav(BinaryStreamReader *f)
{
    if (!f->IsOpen())
        return false;

    m_filename = StringDuplicate(f->m_filename);


    // 
    // Read header

    unsigned char buf1[4];
    if (f->ReadBytes(4, buf1) != 4 || memcmp(buf1, "RIFF", 4) != 0)
        return false;

    unsigned chunkSize = f->ReadU32();

    if (f->ReadBytes(4, buf1) != 4 || memcmp(buf1, "WAVE", 4) != 0)
        return false;

    //
    // Read fmt chunk

    if (f->ReadBytes(4, buf1) != 4 || memcmp(buf1, "fmt ", 4) != 0)
        return false;
    unsigned fmtChunkSize = f->ReadU32();
    unsigned audioFormat = f->ReadU16();
    m_numChannels = f->ReadU16();
    unsigned sampleRate = f->ReadU32();
    unsigned byteRate = f->ReadU32();
    unsigned bytesPerGroup = f->ReadU16();
    unsigned bitsPerSample = f->ReadU16();

    ReleaseAssert(audioFormat == 1, "File '%s' unsupported format", m_filename);
    ReleaseAssert(m_numChannels == 2, "File '%s' is not stereo", m_filename);
    ReleaseAssert(bytesPerGroup == 4, "File '%s' unsupported block alignment", m_filename);
    ReleaseAssert(bitsPerSample == 16, "File '%s' is not 16 bits sample depth", m_filename);

    if (fmtChunkSize == 20)
        f->ReadU32(); // Skip extra 4 bytes of data that isn't normally present and isn't useful.


    //
    // Read data chunk

    if (f->ReadBytes(4, buf1) != 4 || memcmp(buf1, "data", 4) != 0)
        return false;
    unsigned dataChunkSize = f->ReadU32();
    ReleaseAssert(dataChunkSize % bytesPerGroup == 0, "File '%s' ends with half a sample", m_filename);
    unsigned numGroups = dataChunkSize / bytesPerGroup;
    unsigned numBlocks = numGroups / SampleBlock::MAX_SAMPLES;
    if (numGroups % SampleBlock::MAX_SAMPLES != 0)
        numBlocks++;

    m_channels = new SoundChannel* [m_numChannels];
    for (int i = 0; i < m_numChannels; i++)
        m_channels[i] = new SoundChannel;

    int16_t *buf = new int16_t [SampleBlock::MAX_SAMPLES * m_numChannels];

    for (int blockCount = 0; blockCount < numBlocks; blockCount++)
    {
        unsigned bytesToRead = bytesPerGroup * SampleBlock::MAX_SAMPLES;
        if (blockCount + 1 == numBlocks)
            bytesToRead = (numGroups % SampleBlock::MAX_SAMPLES) * bytesPerGroup;
        size_t bytesRead = f->ReadBytes(bytesToRead, (unsigned char *)buf);
        size_t groupsRead = bytesRead / bytesPerGroup;

        for (int chan_idx = 0; chan_idx < m_numChannels; chan_idx++)
        {
            SoundChannel *chan = m_channels[chan_idx];
            SampleBlock *block = new SampleBlock;

            for (size_t i = 0; i < groupsRead; i++)
                block->m_samples[i] = buf[i * m_numChannels + chan_idx];

            block->m_len = groupsRead;
            block->RecalcLuts();

            DebugAssert(block->m_len > 0);
            chan->m_blocks.Push(block);

            DebugAssert((block->m_len == SampleBlock::MAX_SAMPLES) || (blockCount + 1 == numBlocks));
        }
    }

    delete[] buf;

    return true;
}


bool Sound::SaveWav()
{
    BinaryFileWriter f(m_filename);
    if (!f.m_file)
        return false;

    return SaveWav(&f, 0, -1);
}


bool Sound::SaveWav(BinaryStreamWriter *f, int64_t startIdx, int64_t endIdx)
{
    if (endIdx < 0)
        endIdx = GetLength() - 1;
	
    unsigned const SIZE_OF_HEADERS = 36;
    unsigned const BYTES_PER_GROUP = m_numChannels * 2;
    unsigned const NUM_SAMPLES_TO_OUTPUT = (endIdx - startIdx + 1);
    unsigned const SIZE_OF_DATA = NUM_SAMPLES_TO_OUTPUT * BYTES_PER_GROUP;
    f->Reserve(SIZE_OF_HEADERS + SIZE_OF_DATA + 8);


    // 
    // Write header

    f->WriteBytes("RIFF", 4);
    f->WriteU32(SIZE_OF_HEADERS + SIZE_OF_DATA);    // Chunk size
    f->WriteBytes("WAVE", 4);


    //
    // Write fmt chunk

    f->WriteBytes("fmt ", 4);

    f->WriteU32(16);                         // fmtChunkSize
    f->WriteU16(1);                          // Audio format. 1=PCM.
    f->WriteU16(m_numChannels);
    f->WriteU32(44100);                      // Sample rate
    f->WriteU32(44100 * BYTES_PER_GROUP);    // Byte rate
    f->WriteU16(BYTES_PER_GROUP);
    f->WriteU16(16);                         // Bits per sample


    //
    // Write data chunk

    f->WriteBytes("data", 4);
    f->WriteU32(SIZE_OF_DATA);               // Data chunk size

    int16_t *buf = new int16_t[SampleBlock::MAX_SAMPLES * m_numChannels];

    SoundChannel::SoundPos pos = m_channels[0]->GetSoundPosFromSampleIdx(startIdx);
    int64_t samplesLeftToOutput = NUM_SAMPLES_TO_OUTPUT;
    while (samplesLeftToOutput > 0)
    {
        int len = m_channels[0]->m_blocks[pos.m_blockIdx]->m_len;
        if (len > samplesLeftToOutput)
            len = samplesLeftToOutput;

        for (int chan_idx = 0; chan_idx < m_numChannels; chan_idx++)
        {
            SoundChannel *chan = m_channels[chan_idx];
            SampleBlock *block = chan->m_blocks[pos.m_blockIdx];

            for (size_t i = 0; i < len; i++)
                buf[i * m_numChannels + chan_idx] = block->m_samples[i];
        }

        f->WriteBytes((char *)buf, len * BYTES_PER_GROUP);
        samplesLeftToOutput -= len;
    }

    delete[] buf;

    return true;
}


int64_t Sound::GetLength()
{
    if (m_numChannels == 0)
        return 0;

    if (m_cachedLength < 0)
        m_cachedLength = m_channels[0]->GetLength();

    return m_cachedLength;
}
