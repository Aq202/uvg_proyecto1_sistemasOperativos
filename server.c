// Server side C program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <asm-generic/socket.h>
#include <pthread.h> 
#define PORT 8080
#define BUFFER_SIZE 1

void *thread_listening_client(void *param) {
    int socket_id = *((int *)param);
    printf("Id del socket del cliente: %d\n", socket_id);


    char buffer[BUFFER_SIZE];
	char *message_received = NULL;
    ssize_t bytes_read;
	size_t total_bytes_received = 0;

    while (1) {
        // Leer x bytes del socket
        bytes_read = recv(socket_id, &buffer, BUFFER_SIZE, 0);

		// Si se cierra la conexión o hay error, dejar de esperar
        if (bytes_read <= 0) {
            break;
        }

		// Realizar 'append' de bytes leidos
		message_received = (char *)realloc(message_received, total_bytes_received + bytes_read);
		memcpy(message_received + total_bytes_received, buffer, bytes_read);
        total_bytes_received += bytes_read;

		printf("El mensaje recibido es: %s\n", message_received);
    }

	printf("Conexión %d ha sido cerrada.\n", socket_id);

    // liberar memoria, Cerrar el socket y salir del hilo
	free(message_received);
    close(socket_id);
    pthread_exit(NULL);
}

int main(int argc, char const* argv[])
{
	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	socklen_t addrlen = sizeof(address);
	

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

    pthread_t thread_id;
    while(1){

        // Aceptar conexion
        if ((new_socket
            = accept(server_fd, (struct sockaddr*)&address,
                    &addrlen))
            < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Crear thread para conexión aceptada
        pthread_create(&thread_id, NULL, thread_listening_client, (void *)&new_socket);

        
    }

    pthread_join(thread_id, NULL);
	// closing the listening socket
	close(server_fd);
	return 0;
}
