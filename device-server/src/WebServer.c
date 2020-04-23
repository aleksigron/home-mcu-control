#include "WebServer.h"

#include <string.h>

#include "civetweb/civetweb.h"

static int handler(struct mg_connection* conn, void* ignored)
{
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

int WebServer_init(struct WebServer* server)
{
    /* Initialize the library */
    mg_init_library(0);

    /* Start the server */
    server->context = mg_start(NULL, NULL, NULL);

    /* Add some handler */
    mg_set_request_handler(server->context, "/hello", handler, "Hello world");

	return 0;
}

int WebServer_deinit(struct WebServer* server)
{
    /* Stop the server */
    mg_stop(server->context);

	server->context = NULL;

    /* Un-initialize the library */
    mg_exit_library();

	return 0;
}
