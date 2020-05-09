#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

struct DeviceRequest;

struct DeviceServer
{
	int listenSocket;
	int openSocket;

	pthread_mutex_t requestMutex;

	struct DeviceRequest* requestQueue;
	size_t requestQueueStart;
	size_t requestQueueCount;
	size_t requestQueueCapacity;
};

int DeviceServer_run(struct DeviceServer* server);
bool DeviceServer_requestReceive(struct DeviceServer* server, struct DeviceRequest* request);
