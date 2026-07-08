#include "Storage.h"

Storage::Storage(const String& name)
{
    namespaceName = name;
    opened = false;
}

bool Storage::begin()
{
    // باز کردن Namespace در حالت خواندن/نوشتن
    opened = preferences.begin(namespaceName.c_str(), false);
    return opened;
}

void Storage::end()
{
    // بستن ارتباط با NVS
    if (opened)
    {
        preferences.end();
        opened = false;
    }
}

bool Storage::isOpen()
{
    return opened;
}

bool Storage::putBool(const char* key, bool value)
{
    if (!opened) return false;
    return preferences.putBool(key, value) > 0;
}

bool Storage::getBool(const char* key, bool defaultValue)
{
    if (!opened) return defaultValue;
    return preferences.getBool(key, defaultValue);
}

bool Storage::putInt(const char* key, int32_t value)
{
    if (!opened) return false;
    return preferences.putInt(key, value) > 0;
}

int32_t Storage::getInt(const char* key, int32_t defaultValue)
{
    if (!opened) return defaultValue;
    return preferences.getInt(key, defaultValue);
}

bool Storage::putUInt(const char* key, uint32_t value)
{
    if (!opened) return false;
    return preferences.putUInt(key, value) > 0;
}

uint32_t Storage::getUInt(const char* key, uint32_t defaultValue)
{
    if (!opened) return defaultValue;
    return preferences.getUInt(key, defaultValue);
}

bool Storage::putFloat(const char* key, float value)
{
    if (!opened) return false;
    return preferences.putFloat(key, value) > 0;
}

float Storage::getFloat(const char* key, float defaultValue)
{
    if (!opened) return defaultValue;
    return preferences.getFloat(key, defaultValue);
}

bool Storage::putString(const char* key, const String& value)
{
    if (!opened) return false;
    return preferences.putString(key, value) > 0;
}

String Storage::getString(const char* key, const String& defaultValue)
{
    if (!opened) return defaultValue;
    return preferences.getString(key, defaultValue);
}

bool Storage::putBytes(const char* key, const void* data, size_t length)
{
    // برای ذخیره کدهای RF / IR یا داده خام
    if (!opened) return false;
    if (data == nullptr || length == 0) return false;

    return preferences.putBytes(key, data, length) == length;
}

size_t Storage::getBytes(const char* key, void* buffer, size_t maxLength)
{
    // خواندن داده خام داخل بافر
    if (!opened) return 0;
    if (buffer == nullptr || maxLength == 0) return 0;

    return preferences.getBytes(key, buffer, maxLength);
}

size_t Storage::getBytesLength(const char* key)
{
    // فهمیدن اندازه داده ذخیره‌شده قبل از خواندن
    if (!opened) return 0;

    return preferences.getBytesLength(key);
}

bool Storage::remove(const char* key)
{
    if (!opened) return false;
    return preferences.remove(key);
}

bool Storage::clear()
{
    // پاک کردن همه کلیدهای همین Namespace
    if (!opened) return false;
    return preferences.clear();
}

bool Storage::exists(const char* key)
{
    if (!opened) return false;
    return preferences.isKey(key);
}

bool Storage::factoryReset()
{
    // فعلاً معادل clear است؛ بعداً اگر چند Namespace داشتیم توسعه می‌دهیم
    return clear();
}