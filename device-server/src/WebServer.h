#pragma once

struct mg_context;

struct WebServer {
	struct mg_context* context;
};

int WebServer_init(struct WebServer* server);
int WebServer_deinit(struct WebServer* server);
