#ifndef CONFIG_KEY_H
#define CONFIG_KEY_H

#include <ConfigCommon.h>

class ConfigKey
{
public:
    ConfigKey();
    bool set(const char* value);
    bool isValid() const;
    const char* c_str() const;
    size_t length() const;
    bool equals(const ConfigKey& other) const;

private:
    char value_[CONFIG_KEY_MAX_LENGTH];
};

#endif
