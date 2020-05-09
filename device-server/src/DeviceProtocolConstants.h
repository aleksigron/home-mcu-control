#pragma once

enum MsgPos
{
	MsgPos_Protocol = 0,
	MsgPos_Length = 4,
	MsgPos_Number = 5,
	MsgPos_Type = 7,

	// MsgType_Acknowledge
	MsgPos_ResponseTo = 8,

	// MsgType_Connected & MsgType_SetName
	MsgPos_DeviceName = 8,

	// MsgType_SetLighting
	MsgPos_Animation = 8,
	MsgPos_AnimationSpeed = 9,
	MsgPos_Hue = 10,
	MsgPos_Brightness = 11
};

enum MsgType
{
	MsgType_None = 0,
	MsgType_Acknowledge = 1,
	MsgType_Connected = 2,
	MsgType_SetName = 3,
	MsgType_SetLighting = 4
};

enum MsgLen
{
	MsgLen_Connected = 10,
	MsgLen_Acknowledge = 10,
	MsgLen_SetName = 10,
	MsgLen_SetLighting = 12
};

enum AnimType
{
	AnimType_None = 0,
	AnimType_BrightnessPulse = 1,
	AnimType_BrightnessWave = 2
};
