#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <Preferences.h>

class Storage
{
private:
    // شیء اصلی NVS در ESP32
    Preferences preferences;

    // وضعیت باز بودن حافظه
    bool opened;

    // نام فضای ذخیره‌سازی
    String namespaceName;

public:
    Storage(const String& name = "app");

    bool begin();
    void end();
    bool isOpen();

    bool putBool(const char* key, bool value);
    bool getBool(const char* key, bool defaultValue = false);

    bool putInt(const char* key, int32_t value);
    int32_t getInt(const char* key, int32_t defaultValue = 0);

    bool putUInt(const char* key, uint32_t value);
    uint32_t getUInt(const char* key, uint32_t defaultValue = 0);

    bool putFloat(const char* key, float value);
    float getFloat(const char* key, float defaultValue = 0.0);

    bool putString(const char* key, const String& value);
    String getString(const char* key, const String& defaultValue = "");

    // ذخیره داده خام باینری، مناسب RF / IR / داده‌های خاص
    bool putBytes(const char* key, const void* data, size_t length);

    // خواندن داده خام باینری
    size_t getBytes(const char* key, void* buffer, size_t maxLength);

    // گرفتن اندازه داده ذخیره‌شده
    size_t getBytesLength(const char* key);

    bool remove(const char* key);
    bool clear();
    bool exists(const char* key);

    // پاک‌سازی کامل تنظیمات همین Namespace
    bool factoryReset();
};

#endif