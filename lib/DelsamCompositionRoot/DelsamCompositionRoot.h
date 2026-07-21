#ifndef DELSAM_COMPOSITION_ROOT_H
#define DELSAM_COMPOSITION_ROOT_H

#include <stdint.h>

class DelsamCompositionRoot
{
public:
    static bool begin(uint32_t nowMs);
    static void update(uint32_t nowMs);
    static bool isReady();

private:
    DelsamCompositionRoot() = delete;
};

#endif
