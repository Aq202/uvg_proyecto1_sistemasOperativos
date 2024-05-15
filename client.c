// Client side C program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "chat.pb-c.h"
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h> 
#define PORT 8080
#define BUFFER_SIZE 1

void *thread_listening_server(void *param) {

	int socket_id = *((int *)param);
    char buffer[BUFFER_SIZE];
	uint8_t *message_received = NULL;
    ssize_t bytes_read;
	size_t total_bytes_received = 0;

    while (1) {

		bool message_end = false;

        // Leer x bytes del socket
        bytes_read = recv(socket_id, &buffer, BUFFER_SIZE, 0);

		// Si se cierra la conexión o hay error, dejar de esperar
        if (bytes_read <= 0) {
            break;
        }

		message_end = buffer[bytes_read - 1] == '\0';

		if(message_end){
			bytes_read -= 1; // Excluir byte de terminación
		}

		// Realizar 'append' de bytes leidos
		message_received = (char *)realloc(message_received, total_bytes_received + bytes_read);
		memcpy(message_received + total_bytes_received, buffer, bytes_read);
        total_bytes_received += bytes_read;

		if(message_end){

			printf("El mensaje recibido es: %s\n", (char*) message_received);

			// Convertir a objeto request
			Chat__Response *request = chat__response__unpack(NULL, total_bytes_received, message_received);

			if(request != NULL){

				
			}

			total_bytes_received = 0; // Reiniciar cuenta de tamaño de mensaje
		}
		

    }

	printf("Conexión %d ha sido cerrada.\n", socket_id);

    // liberar memoria, Cerrar el socket y salir del hilo
	free(message_received);
    close(socket_id);
    pthread_exit(NULL);
}

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

	Chat__Request request = CHAT__REQUEST__INIT;
	request.operation = CHAT__OPERATION__REGISTER_USER;

	Chat__NewUserRequest new_user_req = CHAT__NEW_USER_REQUEST__INIT;
	new_user_req.username = "Maria";

	request.register_user = &new_user_req;
	request.payload_case = CHAT__REQUEST__PAYLOAD_REGISTER_USER;
    
    size_t buffer_size = chat__request__get_packed_size(&request);

    // Asignar memoria para el buffer
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);

    // Serializar la estructura de mensaje en el buffer
    chat__request__pack(&request, buffer);
	printf("Buffer: %s\n", (char *)buffer);

	send(client_fd, buffer + '\0', buffer_size + 1, 0);
	printf("User message sent\n");
	/*
    valread = read(client_fd, buffer,
				1024 - 1); // subtract 1 for the null
							// terminator at the end
	printf("%s\n", buffer);
    */


   	// Crear hilo para espera de mensajes continua
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_listening_server, (void *)&client_fd);



    pthread_join(thread_id, NULL);

	// closing the connected socket
	close(client_fd);
	return 0;
}
