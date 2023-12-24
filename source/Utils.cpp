#include "Utils.hpp"

namespace Utils
{
    float tickToSeconds(u64 ticks)
    {
        return ((float)((u64)(1000000 * ticks / SYSCLOCK_ARM11)) / 1000000);
    }

    int fpsCounter(void)
    {
        static int fps = 0, counter = 0;
        static u64 lastTick = 0;

        u64 current = svcGetSystemTick();

        if(tickToSeconds(current - lastTick) >= 1.0f)
        {
            fps = counter;
            counter = 0;
            lastTick = current;
        }

        counter++;

        return fps;
    }
} // namespace Utils
