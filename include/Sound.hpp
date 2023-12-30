#pragma once

#include <3ds.h>
#include <cwav.h>
#include <string>

class Sound
{
public:
    Sound(const std::string &filename);
    ~Sound();

    Sound(const Sound &) = delete;
    Sound &operator=(const Sound &) = delete;

    void play(bool forceReset = false);
    void stop(void);
    bool isPlaying(void);
    cwavStatus_t getStatus(void);

    static void initEnvironment(void);
    static void exitEnvironment(void);

private:
    CWAV m_cwav;
    u8 *m_buffer;
    bool m_loaded;
};