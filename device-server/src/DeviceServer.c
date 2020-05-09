#include "DeviceServer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "DeviceProtocol.h"
#include "DeviceProtocolConstants.h"
#include "DeviceRequest.h"
#include "MessageTypes.h"

enum { BufferSize = 128 };
const uint16_t port = 3123;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

static void init(struct DeviceServer* server)
{
	server->listenSocket = 0;
	server->openSocket = 0;

	server->requestQueueCapacity = 8;
	server->requestQueue = malloc(sizeof(struct DeviceRequest) * server->requestQueueCapacity);
	server->requestQueueStart = 0;
	server->requestQueueCount = 0;

	pthread_mutex_init(&server->requestMutex, NULL);
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

	// Set socket to non-blocking mode
    fcntl(server->openSocket, F_SETFL, O_NONBLOCK);
}

static void deinit(struct DeviceServer* server)
{
	close(server->openSocket);
	close(server->listenSocket);

	free(server->requestQueue);

	pthread_mutex_destroy(&server->requestMutex);
}

int DeviceServer_run(struct DeviceServer* server)
{
	unsigned char buffer[BufferSize];
	unsigned short currentMessageNumber = 0;

	init(server);

	initListenSocket(server);

	while (1)
	{
		waitForConnection(server);

		while (1)
		{
			/* Read whatever has been sent to this socket */

			ssize_t readBytes;
			bzero(buffer, BufferSize);
			readBytes = read(server->openSocket, buffer, BufferSize - 1);

			if (readBytes >= 0)
			{
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
			}
			// Ignore if error was that there was no data to read yet
			else if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				printf("Error reading from socket: %s\n", strerror(errno));
				break;
			}

			// Let's sleep a bit to not use 100 % of CPU
			usleep(10000);

			/* Send all requests from our request queue */

			while (1)
			{
				bool requestFound = false;
				struct DeviceRequest request;

				pthread_mutex_lock(&server->requestMutex);

				if (server->requestQueueCount > 0)
				{
					requestFound = true;
					request = server->requestQueue[server->requestQueueStart];

					server->requestQueueStart = (server->requestQueueStart + 1) % server->requestQueueCapacity;
					server->requestQueueCount -= 1;
				}

				pthread_mutex_unlock(&server->requestMutex);

				if (requestFound == true)
				{
					ssize_t res;

					if (request.type == DeviceRequestType_SetLighting)
					{
						struct MessageSetLighting message;

						message.number = currentMessageNumber;
						message.animationType = (uint8_t)(rand() % 3);
						message.animationSpeed = 5;
						message.hue = (uint8_t)(rand() % 256);
						message.brightness = (uint8_t)(rand() % 32);

						printf("Sending SetLighting, animation %d, hue %d, brightness %d\n",
							   (int)message.animationType,
							   (int)message.hue,
							   (int)message.brightness);

						DeviceProtocol_writeMessageSetLighting(buffer, &message);

						currentMessageNumber += 1;

						res = write(server->openSocket, &buffer, MsgLen_SetLighting);

						if (res == -1)
						{
							printf("Error writing to socket: %s\n", strerror(errno));
							break;
						}
					}
				}
				else // No request found, break out of loop
				{
					break;
				}
			}
		}
	}

	deinit(server);


	return 0;
}

bool DeviceServer_requestReceive(struct DeviceServer* server, struct DeviceRequest* request)
{
	bool result;

	pthread_mutex_lock(&server->requestMutex);

	if (server->requestQueueCount < server->requestQueueCapacity)
	{
		size_t index = (server->requestQueueStart + server->requestQueueCount) % server->requestQueueCapacity;
		server->requestQueue[index] = *request;
		server->requestQueueCount += 1;

		result = true;
	}
	else
	{
		result = false;
	}

	pthread_mutex_unlock(&server->requestMutex);

	return result;
}
