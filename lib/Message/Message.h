// lib/Message/Message.h

#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>

// نوع کلی پیام، بدون وابستگی به Relay، Sensor، RF، IR یا هر Device خاص
enum class MessageType : uint8_t
{
    None = 0,
    Command,
    Event,
    Response,
    Discovery,
    Heartbeat,
    Error
};

// ساختار استاندارد پیام بین نودها
struct Message
{
    // شناسه نود ارسال‌کننده
    uint16_t sourceNodeId = 0;

    // شناسه نود مقصد
    uint16_t destinationNodeId = 0;

    // نوع کلی پیام
    MessageType type = MessageType::None;

    // شناسه پیام برای تطبیق درخواست و پاسخ
    uint16_t messageId = 0;

    // آیا فرستنده انتظار پاسخ دارد؟
    bool requireAck = false;

    // اندازه داده واقعی داخل payload
    uint8_t payloadLength = 0;

    // داده خام پیام؛ معنی آن را کلاس‌های بالاتر تفسیر می‌کنند
    uint8_t payload[64] = {0};
};

#endif