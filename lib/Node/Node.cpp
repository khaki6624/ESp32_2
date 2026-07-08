#include "Node.h"

Node::Node(uint16_t id)
{
    nodeId = id;

    name = "";

    firmwareVersion = "";

    hardwareVersion = "";

    online = false;

    lastSeen = 0;

    transport = TransportType::None;

    address = 0;

    outputCount = 0;
    inputCount = 0;
    sensorCount = 0;
}

void Node::begin()
{
}

void Node::update()
{
}

void Node::setNodeId(uint16_t id)
{
    nodeId = id;
}

uint16_t Node::getNodeId()
{
    return nodeId;
}

void Node::setName(const String& value)
{
    name = value;
}

String Node::getName()
{
    return name;
}

void Node::setFirmwareVersion(const String& version)
{
    firmwareVersion = version;
}

String Node::getFirmwareVersion()
{
    return firmwareVersion;
}

void Node::setHardwareVersion(const String& version)
{
    hardwareVersion = version;
}

String Node::getHardwareVersion()
{
    return hardwareVersion;
}

void Node::setOnline(bool value)
{
    online = value;
}

bool Node::isOnline()
{
    return online;
}

void Node::updateLastSeen()
{
    lastSeen = millis();
}

uint32_t Node::getLastSeen()
{
    return lastSeen;
}

void Node::setTransport(TransportType type)
{
    transport = type;
}

TransportType Node::getTransport()
{
    return transport;
}

void Node::setAddress(uint16_t addr)
{
    address = addr;
}

uint16_t Node::getAddress()
{
    return address;
}

void Node::setOutputCount(uint16_t count)
{
    outputCount = count;
}

uint16_t Node::getOutputCount()
{
    return outputCount;
}

void Node::setInputCount(uint16_t count)
{
    inputCount = count;
}

uint16_t Node::getInputCount()
{
    return inputCount;
}

void Node::setSensorCount(uint16_t count)
{
    sensorCount = count;
}

uint16_t Node::getSensorCount()
{
    return sensorCount;
}