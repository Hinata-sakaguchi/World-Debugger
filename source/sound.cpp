#include "Sound.hpp"

#include <stdio.h>
#include <string.h>

Sound::Sound(const std::string &filename) : m_buffer(nullptr), m_loaded(false)
{
    u32 size;
    FILE *fp = fopen(filename.c_str(), "rb");

    if (!fp)
        return;

    // Get file size
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Read file
    m_buffer = (u8 *)linearAlloc(size);
    fread(m_buffer, 1, size, fp);
    fclose(fp);

    // Init cwav
    memset(&m_cwav, 0, sizeof(CWAV));
    cwavLoad(&m_cwav, m_buffer, 1);
    m_cwav.dataBuffer = m_buffer;

    m_loaded = true;
}

Sound::~Sound(void)
{
    cwavFree(&m_cwav);

    if (m_buffer)
    {
        linearFree(m_buffer);
        m_buffer = nullptr;
    }
}

void Sound::initEnvironment(void)
{
    ndspInit();
    cwavUseEnvironment(CWAV_ENV_DSP);
}

void Sound::exitEnvironment(void)
{
    ndspExit();
}

void Sound::play(bool forceReset)
{
    if (!m_loaded)
        return;

    if (isPlaying())
    {
        if (!forceReset)
            return;

        stop();
    }

    if (m_cwav.numChannels == 1)
        cwavPlay(&m_cwav, 0, -1);
    else
        cwavPlay(&m_cwav, 0, 1);
}

void Sound::stop(void)
{
    if (!isPlaying())
        return;

    if (m_cwav.numChannels == 1)
        cwavStop(&m_cwav, 0, -1);
    else
        cwavStop(&m_cwav, 0, 1);
}

bool Sound::isPlaying(void)
{
    return cwavIsPlaying(&m_cwav);
}

cwavStatus_t Sound::getStatus(void)
{
    return m_cwav.loadStatus;
}