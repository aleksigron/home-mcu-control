#pragma once

const uint8_t ProtocolIdentityLength = 4;
const uint8_t ProtocolIdentity[ProtocolIdentityLength] = { 101, 232, 25, 164 };

enum
{
	MsgPos_Protocol = 0,
	MsgPos_Length = 4,
	MsgPos_Number = 5,
	MsgPos_Type = 7,

	// For MessageType_Set
	MsgPos_Animation = 8,
	MsgPos_AnimationSpeed = 9,
	MsgPos_Hue = 10,
	MsgPos_Brightness = 11,

	// For MessageType_Connected
	MsgPos_DeviceName = 8,

	// For MessageType_Acknowledge
	MsgPos_ResponseTo = 8
};

enum
{
	MessageType_None = 0,
	MessageType_Acknowledge = 1,
	MessageType_Connected = 2,
	MessageType_Set = 3
};

enum
{
	MessageLength_Connected = 9,
	MessageLength_Acknowledge = 10,
	MessageLength_Set = 12
};

enum
{
	AnimationType_None = 0,
	AnimationType_BrightnessPulse = 1,
	AnimationType_BrightnessWave = 2
};

void writeProtocolIdentity(uint8_t* array)
{
	for (int i = 0; i < ProtocolIdentityLength; ++i)
	{
		array[i] = ProtocolIdentity[i];
	}
}

void uint16ToUint8Array(uint16_t val, uint8_t* arr)
{
	arr[0] = (uint8_t)(val & 0x00ff);
	arr[1] = (uint8_t)((val & 0xff00) >> 8);
}

uint16_t uint8ArrayToUint16(const uint8_t* arr)
{
	uint16_t result = arr[0];
	result |= arr[1] << 8;
	return result;
}
