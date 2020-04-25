#include "DeviceProtocol.h"

#include "DeviceProtocolConstants.h"
#include "MessageTypes.h"

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

const uint8_t* DeviceProtocol_findMessageStart(const uint8_t* buffer, const uint8_t* end)
{
	size_t identityBytesFound = 0;
	const uint8_t* messageStart = buffer;

	// Find start of proper message
	while (identityBytesFound < ProtocolIdentityLength && buffer != end)
	{
		if (*buffer == ProtocolIdentity[identityBytesFound])
		{
			identityBytesFound += 1;
		}
		else
		{
			identityBytesFound = 0;
			messageStart = buffer + 1;
		}

		buffer += 1;
	}

	if (identityBytesFound == ProtocolIdentityLength)
	{
		return messageStart;
	}
	else
	{
		return NULL;
	}
}

uint8_t DeviceProtocol_getMessageType(const uint8_t* messageStart)
{
	return messageStart[MsgPos_Type];
}

uint8_t DeviceProtocol_getMessageLength(const uint8_t* messageStart)
{
	return messageStart[MsgPos_Length];
}

void DeviceProtocol_readMessageAcknowledge(const uint8_t* message, struct MessageAcknowledge* messageOut)
{
	messageOut->number = DeviceProtocol_uint8ArrayToUint16(&message[MsgPos_Number]);
	messageOut->responseTo = DeviceProtocol_uint8ArrayToUint16(&message[MsgPos_ResponseTo]);
}

void DeviceProtocol_readMessageConnected(const uint8_t* message, struct MessageConnected* messageOut)
{
	messageOut->number = DeviceProtocol_uint8ArrayToUint16(&message[MsgPos_Number]);
	messageOut->deviceName = DeviceProtocol_uint8ArrayToUint16(&message[MsgPos_DeviceName]);
}

void DeviceProtocol_readMessageSet(const uint8_t* message, struct MessageSet* messageOut)
{
	messageOut->number = DeviceProtocol_uint8ArrayToUint16(&message[MsgPos_Number]);

	messageOut->animationType = message[MsgPos_Animation];
	messageOut->animationSpeed = message[MsgPos_AnimationSpeed];
	messageOut->hue = message[MsgPos_Hue];
	messageOut->brightness = message[MsgPos_Brightness];
}

void DeviceProtocol_writeIdentity(uint8_t* buffer)
{
	for (int i = 0; i < ProtocolIdentityLength; ++i)
	{
		buffer[i] = ProtocolIdentity[i];
	}
}
void DeviceProtocol_writeMessageSet(uint8_t* buffer, uint16_t number, uint8_t animType,
	uint8_t animSpeed, uint8_t hue, uint8_t brightness)
{
	DeviceProtocol_writeIdentity(&buffer[0]);
	buffer[MsgPos_Length] = MsgLen_Set;
	DeviceProtocol_uint16ToUint8Array(number, &buffer[MsgPos_Number]);
	buffer[MsgPos_Type] = MsgType_Set;
	buffer[MsgPos_Animation] = animType;
	buffer[MsgPos_AnimationSpeed] = animSpeed;
	buffer[MsgPos_Hue] = hue;
	buffer[MsgPos_Brightness] = brightness;
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
