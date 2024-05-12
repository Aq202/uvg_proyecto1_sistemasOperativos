// Client side C program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "chat.pb-c.h"
#include <stdlib.h>
#define PORT 8080

int main(int argc, char const* argv[])
{
	int status, valread, client_fd;
	struct sockaddr_in serv_addr;

	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((status
		= connect(client_fd, (struct sockaddr*)&serv_addr,
				sizeof(serv_addr)))
		< 0) {
		printf("\nConnection Failed \n");
		return -1;
	}


    Chat__User user = CHAT__USER__INIT;
    user.username = "Diego";
    user.ip_address = "192.165.254.25";
    user.status = CHAT__USER_STATUS__ONLINE;
    
    size_t buffer_size = chat__user__get_packed_size(&user);

    // Asignar memoria para el buffer
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);

    // Serializar la estructura de mensaje en el buffer
    chat__user__pack(&user, buffer);
	printf("Buffer: %s\n", (char *)buffer);

	send(client_fd, buffer, buffer_size, 0);
	printf("User message sent\n");
	/*
    valread = read(client_fd, buffer,
				1024 - 1); // subtract 1 for the null
							// terminator at the end
	printf("%s\n", buffer);
    */
	// closing the connected socket
	close(client_fd);
	return 0;
}
