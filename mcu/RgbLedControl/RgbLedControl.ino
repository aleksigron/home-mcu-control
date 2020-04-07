#include "FastLED.h"
#include "ESP8266WiFi.h"

#include "WifiCredentials.h"

#define NUM_LEDS 8
#define DATA_PIN 4
#define SEND_BUFFER_SIZE 128
#define READ_BUFFER_SIZE 128

const char* host = "192.168.1.47";
const uint16_t port = 3123;

CRGB leds[NUM_LEDS];
uint8_t sendBuffer[SEND_BUFFER_SIZE];
uint8_t readBuffer[READ_BUFFER_SIZE];

enum MessageByte
{
	MessageByte_Type = 0,
	MessageByte_Hue = 1,
	MessageByte_Brightness = 2
};

enum MessageType : uint8_t
{
	MessageType_None = 0,
	MessageType_Acknowledge = 1,
	MessageType_Set = 2
};

void setup()
{
	// Set up LED strip
	FastLED.addLeds<WS2812B, DATA_PIN>(leds, NUM_LEDS);
	FastLED.setBrightness(32);

	Serial.begin(115200);

	Serial.println();
	Serial.printf("Connecting to %s ", wifi_ssid);

	WiFi.mode(WIFI_STA);

	// These are defined in the WifiCredentials.h file
	WiFi.begin(wifi_ssid, wifi_key);

	int status = WiFi.status();
	while (status != WL_CONNECTED)
	{
		if (status == WL_CONNECT_FAILED)
		{
			Serial.println(" connect failed");
			break;
		}
		else if (status == WL_NO_SSID_AVAIL)
		{
			Serial.println(" SSID not available");
			break;
		}

		delay(1000);
		Serial.print(".");

		status = WiFi.status();
	}

	if (status == WL_CONNECTED)
	{
		Serial.println(" connected!");
	}
}

void loop()
{
	static uint8_t currentHue = 0;
	static bool sendAcknowledge = false;
	static WiFiClient client;

	Serial.printf("[Connecting to %s ... ", host);

	if (client.connect(host, port))
	{
		Serial.println("connected]");
	}
	else
	{
		Serial.println("Connect failed");
		delay(1000);
	}

	while (client.available() || client.connected())
	{
		// Update LEDs
		for (uint8_t i = 0; i < NUM_LEDS; ++i)
		{
			leds[i].setHue((currentHue/* + i * 6*/));
		}

		//currentHue += 1;

		FastLED.show();

		delay(100);
		
		// Update network

		if (client.available())
		{
			int bytesAvailable = client.available();
			
			if (bytesAvailable >= 4)
			{
				client.read(readBuffer, 4);
				
				if (readBuffer[MessageByte_Type] == MessageType_Set)
				{
					uint8_t hue = readBuffer[MessageByte_Hue];
					uint8_t brightness = readBuffer[MessageByte_Brightness];
					
					Serial.printf("Received hue %d, brightness %d\n", (int)hue, (int)brightness);

					currentHue = hue;
					FastLED.setBrightness(brightness);

					sendAcknowledge = true;

				}
			}
			
		}
		if (client.connected())
		{
			if (sendAcknowledge && client.availableForWrite() >= 2)
			{
				sendBuffer[MessageByte_Type] = MessageType_Acknowledge;
				sendBuffer[1] = 0;
				sendBuffer[2] = 0;
				sendBuffer[3] = 0;

				client.write(sendBuffer, 4);
				sendAcknowledge = false;

				client.println("Sent MessageType_Acknowledge");
			}
		}

		yield();
	}
}
