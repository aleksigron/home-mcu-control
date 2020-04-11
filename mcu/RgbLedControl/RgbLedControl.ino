#include "FastLED.h"
#include "ESP8266WiFi.h"

#include "WifiCredentials.h"

#define NUM_LEDS 8
#define DATA_PIN 4
#define SEND_BUFFER_SIZE 128
#define READ_BUFFER_SIZE 128

const char* host = "192.168.1.47";
const uint16_t port = 3123;

const uint8_t MessageHeaderLength = 8;
const uint8_t ProtocolIdentityLength = 4;
const uint8_t ProtocolIdentity[ProtocolIdentityLength] = { 101, 232, 25, 164 };

CRGB leds[NUM_LEDS];
uint8_t sendBuffer[SEND_BUFFER_SIZE];
uint8_t readBuffer[READ_BUFFER_SIZE];

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

enum MessageType : uint8_t
{
	MessageType_None = 0,
	MessageType_Acknowledge = 1,
	MessageType_Connected = 2,
	MessageType_Set = 3
};

enum MessageLength : uint8_t
{
	MessageLength_Connected = 10,
	MessageLength_Acknowledge = 10,
	MessageLength_Set = 12
};

enum AnimationType : uint8_t
{
	AnimationType_None = 0,
	AnimationType_BrightnessWave = 1
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

void setup()
{
	// Set up LED strip
	FastLED.addLeds<WS2812B, DATA_PIN>(leds, NUM_LEDS);
	FastLED.setBrightness(32);

	Serial.begin(115200);

	Serial.println();
	Serial.printf("Connecting to WLAN \"%s\" ", wifi_ssid);

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
	static uint16_t currentMessageNumber = 0;
	static uint8_t currentHue = 0;

	static bool sendAcknowledge = false;
	static uint16_t sendAcknowledgeNumber = 0;

	static bool sendConnected = false;

	static WiFiClient client;

	Serial.printf("[Connecting to server %s:%d ... ", host, (int)port);

	if (client.connect(host, port))
	{
		Serial.println("connected]");
		sendConnected = true;
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
			leds[i].setHue(currentHue);
		}

		FastLED.show();

		delay(100);
		
		// Update network

		if (client.available() > 0)
		{
			int bytesAvailable = client.available();
			int readBytes = (bytesAvailable <= READ_BUFFER_SIZE) ? bytesAvailable : READ_BUFFER_SIZE;

			client.read(readBuffer, readBytes);

			Serial.printf("Read bytes: %d\n", readBytes);

			int bufferByteIndex = 0;

			while (bufferByteIndex < readBytes)
			{
				int identityBytesFound = 0;
				int messageStart = 0;

				// Find start of proper message
				while (identityBytesFound < ProtocolIdentityLength && bufferByteIndex < readBytes)
				{
					if (readBuffer[bufferByteIndex] == ProtocolIdentity[identityBytesFound])
					{
						identityBytesFound += 1;
					}
					else
					{
						identityBytesFound = 0;
						messageStart = bufferByteIndex + 1;
					}

					bufferByteIndex += 1;
				}

				if (readBytes - messageStart < MessageHeaderLength)
				{
					Serial.println("Not enough bytes for message header");
				}
				// We found message start
				else if (identityBytesFound == ProtocolIdentityLength)
				{
					uint8_t messageLength = readBuffer[messageStart + MsgPos_Length];
					uint8_t messageType = readBuffer[messageStart + MsgPos_Type];

					uint16_t messageNumber = uint8ArrayToUint16(&readBuffer[messageStart + MsgPos_Number]);

					// TODO: if (messageStart + messageLength > READ_BUFFER_SIZE)

					if (messageType == MessageType_Acknowledge)
					{
						uint16_t responseTo = uint8ArrayToUint16(&readBuffer[messageStart + MsgPos_ResponseTo]);

						Serial.printf("Received acknowledge to message number %d\n", (int)responseTo);
					}

					if (messageType == MessageType_Set)
					{
						uint8_t animation = readBuffer[messageStart + MsgPos_Animation];
						uint8_t speed = readBuffer[messageStart + MsgPos_AnimationSpeed];
						uint8_t hue = readBuffer[messageStart + MsgPos_Hue];
						uint8_t brightness = readBuffer[messageStart + MsgPos_Brightness];
						
						Serial.printf("Received animation %d, speed %d, hue %d, brightness %d\n",
							(int)animation, (int)speed, (int)hue, (int)brightness);

						currentHue = hue;
						FastLED.setBrightness(brightness);

						sendAcknowledge = true;
						sendAcknowledgeNumber = messageNumber;
					}

					bufferByteIndex = messageStart + messageLength;
				}
			}
		}

		if (client.connected())
		{
			if (sendConnected && client.availableForWrite() >= MessageLength_Connected)
			{
				writeProtocolIdentity(&sendBuffer[0]);
				
				sendBuffer[MsgPos_Length] = MessageLength_Connected;
				uint16ToUint8Array(currentMessageNumber, &sendBuffer[MsgPos_Number]);
				sendBuffer[MsgPos_Type] = MessageType_Connected;

				// TODO: Get device name from EEPROM or filesystem
				uint16ToUint8Array(8995, &sendBuffer[MsgPos_DeviceName]);

				client.write(sendBuffer, MessageLength_Connected);
				
				currentMessageNumber += 1;
				sendConnected = false;
				
				Serial.println("Sent MessageType_Connected");
			}

			if (sendAcknowledge && client.availableForWrite() >= MessageLength_Acknowledge)
			{
				writeProtocolIdentity(&sendBuffer[0]);

				sendBuffer[MsgPos_Length] = MessageLength_Acknowledge;
				uint16ToUint8Array(currentMessageNumber, &sendBuffer[MsgPos_Number]);
				sendBuffer[MsgPos_Type] = MessageType_Acknowledge;
				uint16ToUint8Array(sendAcknowledgeNumber, &sendBuffer[MsgPos_ResponseTo]);

				client.write(sendBuffer, MessageLength_Acknowledge);

				currentMessageNumber += 1;
				sendAcknowledge = false;

				Serial.println("Sent MessageType_Acknowledge");
			}
		}

		yield();
	}
}
