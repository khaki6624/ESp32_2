#ifndef DELSAM_APPLICATION_H
#define DELSAM_APPLICATION_H

#include <stdint.h>

class DelsamApplication
{
public:
    static void begin(uint32_t nowMs);
    static void update(uint32_t nowMs);

private:
    DelsamApplication() = delete;
};

#endif
