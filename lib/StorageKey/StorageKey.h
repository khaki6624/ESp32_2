#ifndef STORAGE_KEY_H
#define STORAGE_KEY_H

#include <StorageCommon.h>

class StorageKey
{
public:
    StorageKey();
    bool set(const char* value);
    bool isValid() const;
    const char* c_str() const;
    size_t length() const;
    bool equals(const StorageKey& other) const;

private:
    char value_[STORAGE_KEY_MAX_LENGTH];
};

#endif
