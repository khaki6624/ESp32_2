#ifndef CONFIG_VALIDATOR_H
#define CONFIG_VALIDATOR_H

#include <ConfigDefinition.h>

class ConfigValidator
{
public:
    ConfigResult validate(const ConfigDefinition& definition,
        const ConfigValue& value) const;
};

#endif
