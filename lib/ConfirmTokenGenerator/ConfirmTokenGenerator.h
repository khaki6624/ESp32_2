#ifndef CONFIRM_TOKEN_GENERATOR_H
#define CONFIRM_TOKEN_GENERATOR_H

#include <CommandCommon.h>

class ConfirmTokenGenerator
{
public:
    virtual ~ConfirmTokenGenerator() = default;
    virtual bool generate(ConfirmToken& output) = 0;
};

#endif
