//
//  main.c
//  device-server
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "WebServer.h"

#define BUFFER_SIZE 256

const uint16_t port = 3123;
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

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


int main(int argc, char *argv[])
{
	int sockfd, newsockfd;
	socklen_t clilen;
	unsigned char buffer[BUFFER_SIZE];
	struct sockaddr_in serv_addr, cli_addr;
	ssize_t readBytes, writeBytes;
	unsigned char send_data[32];
	unsigned short currentMessageNumber = 0;
	struct WebServer webServer;

	WebServer_init(&webServer);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
	if (newsockfd < 0)
		error("ERROR on accept");

	while (1)
	{
		bzero(buffer, BUFFER_SIZE);
		readBytes = read(newsockfd, buffer, BUFFER_SIZE - 1);

		printf("Read bytes: %d\n", (int)readBytes);

		int bufferByteIndex = 0;

		while (bufferByteIndex < readBytes)
		{
			int identityBytesFound = 0;
			int messageStart = 0;

			// Find start of proper message
			while (identityBytesFound < ProtocolIdentityLength && bufferByteIndex < readBytes)
			{
				if (buffer[bufferByteIndex] == ProtocolIdentity[identityBytesFound])
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

			// We found message start
			if (identityBytesFound == ProtocolIdentityLength)
			{
				printf("We found message start\n");

				uint8_t messageLength = buffer[messageStart + MsgPos_Length];
				uint8_t messageType = buffer[messageStart + MsgPos_Type];

				uint16_t messageNumber = uint8ArrayToUint16(&buffer[messageStart + MsgPos_Number]);

				// TODO: if (messageStart + messageLength > READ_BUFFER_SIZE)


				if (messageType == MessageType_Acknowledge)
				{
					uint16_t responseTo = uint8ArrayToUint16(&buffer[messageStart + MsgPos_ResponseTo]);

					printf("Received acknowledge to message number %d\n", (int)responseTo);
				}

				if (messageType == MessageType_Connected)
				{
					uint16_t deviceName = uint8ArrayToUint16(&buffer[messageStart + MsgPos_DeviceName]);

					printf("Received connected message from device %d\n", (int)deviceName);
				}

				bufferByteIndex = messageStart + messageLength;
			}
		}

		if (readBytes < 0)
		{
			error("ERROR reading from socket");
			break;
		}

		sleep(5);

		writeProtocolIdentity(&send_data[0]);
		send_data[MsgPos_Length] = MessageLength_Set;
		uint16ToUint8Array(currentMessageNumber, &send_data[MsgPos_Number]);
		send_data[MsgPos_Type] = MessageType_Set;
		send_data[MsgPos_Animation] = (uint8_t)(rand() % 3);
		send_data[MsgPos_AnimationSpeed] = 5;
		send_data[MsgPos_Hue] = (uint8_t)(rand() % 256);
		send_data[MsgPos_Brightness] = (uint8_t)(rand() % 256);

		currentMessageNumber += 1;

		printf("Sending MessageType_Set, animation %d, hue %d, brightness %d\n",
			(int)send_data[MsgPos_Animation],
			(int)send_data[MsgPos_Hue],
			(int)send_data[MsgPos_Brightness]);

		writeBytes = write(newsockfd, &send_data, MessageLength_Set);
		if (writeBytes < 0)
		{
			error("ERROR writing to socket");
			break;
		}
	}

	close(newsockfd);
	close(sockfd);

	WebServer_deinit(&webServer);

	return 0;
}
