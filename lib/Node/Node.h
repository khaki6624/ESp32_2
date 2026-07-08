#ifndef NODE_H
#define NODE_H

#include <Arduino.h>

//-----------------------------------------
// نوع ارتباط نود با سیستم
//-----------------------------------------
enum class TransportType
{
    None,
    RS485,
    WiFi,
    Ethernet,
    CAN,
    BLE
};

class Node
{
private:

    // شناسه یکتای نود
    uint16_t nodeId;

    // نام نود
    String name;

    // نسخه Firmware
    String firmwareVersion;

    // نسخه Hardware
    String hardwareVersion;

    // وضعیت آنلاین بودن نود
    bool online;

    // آخرین زمان دریافت پیام از نود
    uint32_t lastSeen;

    // نوع ارتباط
    TransportType transport;

    // آدرس نود در بستر ارتباطی
    uint16_t address;

    // قابلیت‌های سخت‌افزاری
    uint16_t outputCount;
    uint16_t inputCount;
    uint16_t sensorCount;

public:

    Node(uint16_t id = 0);

    void begin();
    void update();

    //-------------------------
    // Node ID
    //-------------------------
    void setNodeId(uint16_t id);
    uint16_t getNodeId();

    //-------------------------
    // Name
    //-------------------------
    void setName(const String& value);
    String getName();

    //-------------------------
    // Firmware Version
    //-------------------------
    void setFirmwareVersion(const String& version);
    String getFirmwareVersion();

    //-------------------------
    // Hardware Version
    //-------------------------
    void setHardwareVersion(const String& version);
    String getHardwareVersion();

    //-------------------------
    // Online Status
    //-------------------------
    void setOnline(bool value);
    bool isOnline();

    //-------------------------
    // Last Seen
    //-------------------------
    void updateLastSeen();
    uint32_t getLastSeen();

    //-------------------------
    // Transport
    //-------------------------
    void setTransport(TransportType type);
    TransportType getTransport();

    //-------------------------
    // Address
    //-------------------------
    void setAddress(uint16_t addr);
    uint16_t getAddress();

    //-------------------------
    // Capabilities
    //-------------------------
    void setOutputCount(uint16_t count);
    uint16_t getOutputCount();

    void setInputCount(uint16_t count);
    uint16_t getInputCount();

    void setSensorCount(uint16_t count);
    uint16_t getSensorCount();
};

#endif