#include "DeviceProtocol.h"

static const uint8_t ProtocolIdentityLength = 4;
static const uint8_t ProtocolIdentity[ProtocolIdentityLength] = { 101, 232, 25, 164 };

const uint8_t* DeviceProtocol_getProtocolIdentity()
{
	return ProtocolIdentity;
}

size_t DeviceProtocol_getProtocolIdentityLength()
{
	return ProtocolIdentityLength;
}

void DeviceProtocol_writeIdentity(uint8_t* array)
{
	for (int i = 0; i < ProtocolIdentityLength; ++i)
	{
		array[i] = ProtocolIdentity[i];
	}
}

void DeviceProtocol_uint16ToUint8Array(uint16_t val, uint8_t* arr)
{
	arr[0] = (uint8_t)(val & 0x00ff);
	arr[1] = (uint8_t)((val & 0xff00) >> 8);
}

uint16_t DeviceProtocol_uint8ArrayToUint16(const uint8_t* arr)
{
	uint16_t result = arr[0];
	result |= arr[1] << 8;
	return result;
}
