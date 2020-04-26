#include "FastLED.h"
#include "ESP8266WiFi.h"

#include "WifiCredentials.h"
#include "DeviceProtocolConstants.h"

#define M_TAU 6.28318530718

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
	static uint8_t currentAnimStep = 0;
	static uint8_t setAnimation = 0, setAnimSpeed = 0, setHue = 0, setBrightness = 0;

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
		if (setAnimation == AnimType_None)
		{
			FastLED.setBrightness(setBrightness);

			for (uint8_t i = 0; i < NUM_LEDS; ++i)
			{
				leds[i].setHue(setHue);
			}
		}
		else if (setAnimation == AnimType_BrightnessPulse)
		{
			currentAnimStep += setAnimSpeed;

			double factor = cos(currentAnimStep / 256.0 * M_TAU) * 0.5 + 0.5;
			FastLED.setBrightness((uint8_t)(setBrightness * factor));
			
			for (uint8_t i = 0; i < NUM_LEDS; ++i)
			{
				leds[i].setHue(setHue);
			}
		}
		else if (setAnimation == AnimType_BrightnessWave)
		{
			currentAnimStep += setAnimSpeed;

			FastLED.setBrightness(setBrightness);
			
			for (uint8_t i = 0; i < NUM_LEDS; ++i)
			{
				double ledAnimStep = (double)currentAnimStep + (256.0 / NUM_LEDS * i);
				double factor = cos(ledAnimStep / 256.0 * M_TAU) * 0.5 + 0.5;
				leds[i].setHSV(setHue, 255, (uint8_t)(factor * 255));
			}
		}

		FastLED.show();

		delay(50);
		
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

					if (messageType == MsgType_Acknowledge)
					{
						uint16_t responseTo = uint8ArrayToUint16(&readBuffer[messageStart + MsgPos_ResponseTo]);

						Serial.printf("Received acknowledge to message number %d\n", (int)responseTo);
					}

					if (messageType == MsgType_SetLighting)
					{
						setAnimation = readBuffer[messageStart + MsgPos_Animation];
						setAnimSpeed = readBuffer[messageStart + MsgPos_AnimationSpeed];
						setHue = readBuffer[messageStart + MsgPos_Hue];
						setBrightness = readBuffer[messageStart + MsgPos_Brightness];
						
						Serial.printf("Received animation %d, speed %d, hue %d, brightness %d\n",
							(int)setAnimation, (int)setAnimSpeed, (int)setHue, (int)setBrightness);

						sendAcknowledge = true;
						sendAcknowledgeNumber = messageNumber;
					}

					bufferByteIndex = messageStart + messageLength;
				}
			}
		}

		if (client.connected())
		{
			if (sendConnected && client.availableForWrite() >= MsgLen_Connected)
			{
				writeProtocolIdentity(&sendBuffer[0]);
				
				sendBuffer[MsgPos_Length] = MsgLen_Connected;
				uint16ToUint8Array(currentMessageNumber, &sendBuffer[MsgPos_Number]);
				sendBuffer[MsgPos_Type] = MsgType_Connected;

				// TODO: Get device name from EEPROM or filesystem
				uint16ToUint8Array(8995, &sendBuffer[MsgPos_DeviceName]);

				client.write(sendBuffer, MsgLen_Connected);
				
				currentMessageNumber += 1;
				sendConnected = false;
				
				Serial.println("Sent MessageType_Connected");
			}

			if (sendAcknowledge && client.availableForWrite() >= MsgLen_Acknowledge)
			{
				writeProtocolIdentity(&sendBuffer[0]);

				sendBuffer[MsgPos_Length] = MsgLen_Acknowledge;
				uint16ToUint8Array(currentMessageNumber, &sendBuffer[MsgPos_Number]);
				sendBuffer[MsgPos_Type] = MsgType_Acknowledge;
				uint16ToUint8Array(sendAcknowledgeNumber, &sendBuffer[MsgPos_ResponseTo]);

				client.write(sendBuffer, MsgLen_Acknowledge);

				currentMessageNumber += 1;
				sendAcknowledge = false;

				Serial.println("Sent MessageType_Acknowledge");
			}
		}

		yield();
	}
}
