#ifndef TEXT_OUTPUT_H
#define TEXT_OUTPUT_H

#include <stddef.h>
#include <stdint.h>

enum class TextOutputResult : uint8_t
{
    SUCCESS = 0, RETRY_LATER, FAILED, COUNT
};

class TextOutput
{
public:
    virtual ~TextOutput() = default;
    virtual TextOutputResult write(const char* data, size_t length) = 0;
};

#endif
