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

struct DeviceServer
{
	int listenSocket;
	int openSocket;
};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

static void initListenSocket(struct DeviceServer* server)
{
	struct sockaddr_in serverAddr;

	server->listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (server->listenSocket < 0)
		error("ERROR opening socket");

	bzero((char *) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	int bindRes = bind(server->listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	if (bindRes < 0)
		error("ERROR on binding");

	int connectionBacklog = 5;
	listen(server->listenSocket, connectionBacklog);
}

static void waitForConnection(struct DeviceServer* server)
{
	socklen_t clientLen;
	struct sockaddr_in clientAddr;

	clientLen = sizeof(clientAddr);
	server->openSocket = accept(server->listenSocket, (struct sockaddr*)&clientAddr, &clientLen);

	if (server->openSocket < 0)
		error("ERROR on accept");
}

static void closeSockets(struct DeviceServer* server)
{
	close(server->openSocket);
	close(server->listenSocket);
}

int DeviceServer_run()
{
	struct DeviceServer server;
	unsigned char buffer[BufferSize];
	ssize_t readBytes, writeBytes;
	unsigned short currentMessageNumber = 0;

	initListenSocket(&server);

	while (1)
	{
		waitForConnection(&server);

		while (1)
		{
			bzero(buffer, BufferSize);
			readBytes = read(server.openSocket, buffer, BufferSize - 1);

			if (readBytes < 0)
			{
				printf("Error reading from socket: %d\n", (int)readBytes);
				break;
			}

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

			sleep(5);

			{
				struct MessageSetLighting message;
				message.number = currentMessageNumber;
				message.animationType = (uint8_t)(rand() % 3);
				message.animationSpeed = 5;
				message.hue = (uint8_t)(rand() % 256);
				message.brightness = (uint8_t)(rand() % 32);

				DeviceProtocol_writeMessageSetLighting(buffer, &message);

				currentMessageNumber += 1;
			}

			printf("Sending MessageType_Set, animation %d, hue %d, brightness %d\n",
				   (int)buffer[MsgPos_Animation],
				   (int)buffer[MsgPos_Hue],
				   (int)buffer[MsgPos_Brightness]);

			writeBytes = write(server.openSocket, &buffer, MsgLen_SetLighting);
			if (writeBytes < 0)
			{
				printf("Error writing to socket: %d\n", (int)writeBytes);
				break;
			}
		}
	}

	closeSockets(&server);

	return EXIT_SUCCESS;
}
