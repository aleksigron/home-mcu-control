#include <stdlib.h>

#include "DeviceServer.h"
#include "WebServer.h"

int main(int argc, char *argv[])
{
	struct WebServer webServer;
	struct DeviceServer deviceServer;

	WebServer_init(&webServer, &deviceServer);
	DeviceServer_run(&deviceServer);
	WebServer_deinit(&webServer);

	return EXIT_SUCCESS;
}
