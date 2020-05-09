#pragma once

enum DeviceRequestType
{
	DeviceRequestType_SetLighting
};

struct DeviceRequest
{
	enum DeviceRequestType type;
};

