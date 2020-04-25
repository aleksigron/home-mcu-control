#include "DeviceServer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "DeviceProtocol.h"
#include "DeviceProtocolConstants.h"
#include "MessageTypes.h"

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

		const uint8_t* bufferItr = buffer;
		const uint8_t* bufferEnd = buffer + readBytes;

		while (bufferItr < bufferEnd)
		{
			const uint8_t* messageStart = DeviceProtocol_findMessageStart(bufferItr, bufferEnd);

			// We found message start
			if (messageStart != NULL)
			{
				uint8_t messageLength = DeviceProtocol_getMessageLength(messageStart);
				uint8_t messageType = DeviceProtocol_getMessageType(messageStart);

				// TODO: if (messageStart + messageLength > READ_BUFFER_SIZE)


				if (messageType == MsgType_Acknowledge)
				{
					struct MessageAcknowledge acknowledge;
					DeviceProtocol_readMessageAcknowledge(messageStart, &acknowledge);

					printf("Received acknowledge to message %d\n", (int)acknowledge.responseTo);
				}

				if (messageType == MsgType_Connected)
				{
					struct MessageConnected connected;
					DeviceProtocol_readMessageConnected(messageStart, &connected);

					printf("Received connected message from device %d\n", (int)connected.deviceName);
				}

				bufferItr = messageStart + messageLength;
			}
			else
			{
				bufferItr = bufferEnd;
			}
		}

		if (readBytes < 0)
		{
			error("ERROR reading from socket");
			break;
		}

		sleep(5);

		{
			uint8_t animType = (uint8_t)(rand() % 3);
			uint8_t animSpeed = 5;
			uint8_t hue = (uint8_t)(rand() % 256);
			uint8_t brightness = (uint8_t)(rand() % 32);

			DeviceProtocol_writeMessageSet(buffer, currentMessageNumber, animType, animSpeed, hue, brightness);

			currentMessageNumber += 1;
		}

		printf("Sending MessageType_Set, animation %d, hue %d, brightness %d\n",
			(int)buffer[MsgPos_Animation],
			(int)buffer[MsgPos_Hue],
			(int)buffer[MsgPos_Brightness]);

		writeBytes = write(newsockfd, &buffer, MsgLen_Set);
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
