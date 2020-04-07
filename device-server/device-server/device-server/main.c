//
//  main.c
//  device-server
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 256

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[BUFFER_SIZE];
	struct sockaddr_in serv_addr, cli_addr;
	ssize_t n;
	unsigned char send_data[4];
	unsigned int sleep_seconds;

	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
	if (newsockfd < 0)
		error("ERROR on accept");

	while (1)
	{
		send_data[0] = 2;
		send_data[1] = (uint8_t)(rand() % 256);
		send_data[2] = (uint8_t)(rand() % 64);
		send_data[3] = 0;

		printf("Sending message %d, hue %d, brightness %d\n",
			   (int)send_data[0], (int)send_data[1], (int)send_data[2]);

		n = write(newsockfd, &send_data, 4);
		if (n < 0)
		{
			error("ERROR writing to socket");
			break;
		}

		bzero(buffer, BUFFER_SIZE);
		n = read(newsockfd, buffer, BUFFER_SIZE - 1);

		if (n < 0)
		{
			error("ERROR reading from socket");
			break;
		}

		printf("Received message %d\n", (int)buffer[0]);

		sleep_seconds = 3 + (rand() % 8);
		sleep(sleep_seconds);
	}

	close(newsockfd);
	close(sockfd);

	return 0;
}
