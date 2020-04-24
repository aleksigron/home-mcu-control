#include "DeviceServer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "DeviceProtocol.h"

enum { BufferSize = 128 };
const uint16_t port = 3123;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int DeviceServer_run()
{
	int sockfd, newsockfd;
	socklen_t clientLen;
	unsigned char buffer[BufferSize];
	struct sockaddr_in serverAddr, clientAddr;
	ssize_t readBytes, writeBytes;
	unsigned short currentMessageNumber = 0;
	size_t protocolIdentityLength = DeviceProtocol_getProtocolIdentityLength();
	const uint8_t* protocolIdentity = DeviceProtocol_getProtocolIdentity();

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	bzero((char *) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		error("ERROR on binding");

	listen(sockfd,5);
	clientLen = sizeof(clientAddr);
	newsockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientLen);
	if (newsockfd < 0)
		error("ERROR on accept");

	while (1)
	{
		bzero(buffer, BufferSize);
		readBytes = read(newsockfd, buffer, BufferSize - 1);

		printf("Read bytes: %d\n", (int)readBytes);

		int bufferByteIndex = 0;

		while (bufferByteIndex < readBytes)
		{
			int identityBytesFound = 0;
			int messageStart = 0;

			// Find start of proper message
			while (identityBytesFound < protocolIdentityLength && bufferByteIndex < readBytes)
			{
				if (buffer[bufferByteIndex] == protocolIdentity[identityBytesFound])
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
			if (identityBytesFound == protocolIdentityLength)
			{
				printf("We found message start\n");

				uint8_t messageLength = buffer[messageStart + MsgPos_Length];
				uint8_t messageType = buffer[messageStart + MsgPos_Type];

				// TODO: if (messageStart + messageLength > READ_BUFFER_SIZE)


				if (messageType == MessageType_Acknowledge)
				{
					uint16_t responseTo = DeviceProtocol_uint8ArrayToUint16(&buffer[messageStart + MsgPos_ResponseTo]);

					printf("Received acknowledge to message number %d\n", (int)responseTo);
				}

				if (messageType == MessageType_Connected)
				{
					uint16_t deviceName = DeviceProtocol_uint8ArrayToUint16(&buffer[messageStart + MsgPos_DeviceName]);

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

		DeviceProtocol_writeIdentity(&buffer[0]);
		buffer[MsgPos_Length] = MessageLength_Set;
		DeviceProtocol_uint16ToUint8Array(currentMessageNumber, &buffer[MsgPos_Number]);
		buffer[MsgPos_Type] = MessageType_Set;
		buffer[MsgPos_Animation] = (uint8_t)(rand() % 3);
		buffer[MsgPos_AnimationSpeed] = 5;
		buffer[MsgPos_Hue] = (uint8_t)(rand() % 256);
		buffer[MsgPos_Brightness] = (uint8_t)(rand() % 256);

		currentMessageNumber += 1;

		printf("Sending MessageType_Set, animation %d, hue %d, brightness %d\n",
			(int)buffer[MsgPos_Animation],
			(int)buffer[MsgPos_Hue],
			(int)buffer[MsgPos_Brightness]);

		writeBytes = write(newsockfd, &buffer, MessageLength_Set);
		if (writeBytes < 0)
		{
			error("ERROR writing to socket");
			break;
		}
	}

	close(newsockfd);
	close(sockfd);

	return 0;
}
