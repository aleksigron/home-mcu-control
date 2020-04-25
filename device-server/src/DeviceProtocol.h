#pragma once

#include <stddef.h>
#include <stdint.h>

const uint8_t* DeviceProtocol_getProtocolIdentity();
size_t DeviceProtocol_getProtocolIdentityLength();

const uint8_t* DeviceProtocol_findMessageStart(const uint8_t* buffer, const uint8_t* end);

void DeviceProtocol_writeIdentity(uint8_t* buffer);

void DeviceProtocol_writeMessageSet(uint8_t* buffer, uint16_t number, uint8_t animType,
	uint8_t animSpeed, uint8_t hue, uint8_t brightness);

void DeviceProtocol_uint16ToUint8Array(uint16_t val, uint8_t* arr);
uint16_t DeviceProtocol_uint8ArrayToUint16(const uint8_t* arr);
