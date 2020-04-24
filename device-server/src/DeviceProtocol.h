#pragma once

#include <stddef.h>
#include <stdint.h>

enum MsgPos
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

enum MsgType
{
	MessageType_None = 0,
	MessageType_Acknowledge = 1,
	MessageType_Connected = 2,
	MessageType_Set = 3
};

enum MsgLen
{
	MessageLength_Connected = 9,
	MessageLength_Acknowledge = 10,
	MessageLength_Set = 12
};

enum AnimType
{
	AnimationType_None = 0,
	AnimationType_BrightnessPulse = 1,
	AnimationType_BrightnessWave = 2
};

const uint8_t* DeviceProtocol_getProtocolIdentity();
size_t DeviceProtocol_getProtocolIdentityLength();

void DeviceProtocol_writeIdentity(uint8_t* array);

void DeviceProtocol_uint16ToUint8Array(uint16_t val, uint8_t* arr);
uint16_t DeviceProtocol_uint8ArrayToUint16(const uint8_t* arr);
