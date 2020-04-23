#include "DeviceServer.h"
#include "WebServer.h"

int main(int argc, char *argv[])
{
	struct WebServer webServer;

	WebServer_init(&webServer);
	DeviceServer_run();
	WebServer_deinit(&webServer);

	return 0;
}
