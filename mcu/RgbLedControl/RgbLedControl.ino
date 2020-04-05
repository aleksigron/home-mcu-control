#include "FastLED.h"
#define NUM_LEDS 8
#define DATA_PIN 4

CRGB leds[NUM_LEDS];

void setup()
{
	FastLED.addLeds<WS2812B, DATA_PIN>(leds, NUM_LEDS);
	FastLED.setBrightness(32);
}
void loop()
{
	static uint8_t currentHue = 0;

	for (uint8_t i = 0; i < NUM_LEDS; ++i)
	{
		leds[i].setHue((currentHue + i * 6));
	}

	currentHue += 1;

	FastLED.show();
	delay(50);
}
