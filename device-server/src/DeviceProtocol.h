#pragma once

#include <stddef.h>
#include <stdint.h>

struct MessageAcknowledge;
struct MessageConnected;
struct MessageSetName;
struct MessageSetLighting;

const uint8_t* DeviceProtocol_findMessageStart(const uint8_t* buffer, const uint8_t* end);
uint8_t DeviceProtocol_getMessageType(const uint8_t* messageStart);
uint8_t DeviceProtocol_getMessageLength(const uint8_t* messageStart);

void DeviceProtocol_readMessageAcknowledge(const uint8_t* message, struct MessageAcknowledge* messageOut);
void DeviceProtocol_readMessageConnected(const uint8_t* message, struct MessageConnected* messageOut);
void DeviceProtocol_readMessageSet(const uint8_t* message, struct MessageSetLighting* messageOut);

void DeviceProtocol_writeIdentity(uint8_t* buffer);

void DeviceProtocol_writeMessageSetName(uint8_t* buffer, struct MessageSetName* message);
void DeviceProtocol_writeMessageSetLighting(uint8_t* buffer, struct MessageSetLighting* message);

void DeviceProtocol_uint16ToUint8Array(uint16_t val, uint8_t* arr);
uint16_t DeviceProtocol_uint8ArrayToUint16(const uint8_t* arr);
