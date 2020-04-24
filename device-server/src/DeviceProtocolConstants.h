#pragma once

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
	MsgType_None = 0,
	MsgType_Acknowledge = 1,
	MsgType_Connected = 2,
	MsgType_Set = 3
};

enum MsgLen
{
	MsgLen_Connected = 9,
	MsgLen_Acknowledge = 10,
	MsgLen_Set = 12
};

enum AnimType
{
	AnimType_None = 0,
	AnimType_BrightnessPulse = 1,
	AnimType_BrightnessWave = 2
};
