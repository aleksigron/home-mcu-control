#pragma once

struct mg_context;
struct DeviceServer;

struct WebServer {
	struct mg_context* context;
	struct DeviceServer* deviceServer;
};

int WebServer_init(struct WebServer* server, struct DeviceServer* deviceServer);
int WebServer_deinit(struct WebServer* server);
