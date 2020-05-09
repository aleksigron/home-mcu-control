#include "WebServer.h"

#include <string.h>

#include "civetweb/civetweb.h"

#include "DeviceServer.h"
#include "DeviceRequest.h"

static int handler(struct mg_connection* conn, void* serverPtr)
{
	struct WebServer* server = (struct WebServer*)serverPtr;

	// Add a request to be processed by device server

	struct DeviceRequest request;
	request.type = DeviceRequestType_SetLighting;
	DeviceServer_requestReceive(server->deviceServer, &request);

	// Respond to HTTP request

	const char* msg = "Hello world";
	unsigned long len = (unsigned long)strlen(msg);

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Length: %lu\r\n"
	          "Content-Type: text/plain\r\n"
	          "Connection: close\r\n\r\n",
	          len);

	mg_write(conn, msg, len);

	return 200;
}

int WebServer_init(struct WebServer* server, struct DeviceServer* deviceServer)
{
	server->deviceServer = deviceServer;

    /* Initialize the library */
    mg_init_library(0);

    /* Start the server */
    server->context = mg_start(NULL, NULL, NULL);

    /* Add some handler */
    mg_set_request_handler(server->context, "/hello", handler, server);

	return 0;
}

int WebServer_deinit(struct WebServer* server)
{
	server->deviceServer = NULL;

    /* Stop the server */
    mg_stop(server->context);

	server->context = NULL;

    /* Un-initialize the library */
    mg_exit_library();

	return 0;
}
