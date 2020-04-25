#pragma once

#include <stdint.h>

struct MessageAcknowledge
{
	uint16_t number;

	uint16_t responseTo;
};

struct MessageConnected
{
	uint16_t number;

	uint16_t deviceName;
};

struct MessageSetName
{
	uint16_t number;

	uint16_t deviceName;
};

struct MessageSetLighting
{
	uint16_t number;

	uint8_t animationType;
	uint8_t animationSpeed;
	uint8_t hue;
	uint8_t brightness;
};
