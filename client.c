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
#include "consts.h"
#define PORT 8080


void *thread_listening_server(void *param) {

	int socket_id = *((int *)param);
    uint8_t *buffer;

    while (1) {

		buffer = malloc(SOCKET_BUFFER_SIZE);
    	ssize_t bytes_read;

        // Leer x bytes del socket
        bytes_read = read(socket_id, buffer, SOCKET_BUFFER_SIZE);

		// Si se cierra la conexión o hay error, dejar de esperar
        if (bytes_read <= 0) {
			free(buffer);
            break;
        }
		// Redimensionar buffer a tamaño de mensaje leído
		buffer = (uint8_t *)realloc(buffer, bytes_read);

		printf("El mensaje recibido es: %s\n", (char*) buffer);

		// Convertir a objeto request
		Chat__Response *request = chat__response__unpack(NULL, bytes_read, buffer);

		// Liberar memoria de buffer
		free(buffer);

		if(request != NULL){

			
		}		

    }

	printf("Conexión con el servidor ha sido cerrada.\n");

    // Cerrar el socket y salir del hilo
	
    close(socket_id);
    pthread_exit(NULL);
}

int read_number(char *message){
	int num;
    
    // Solicitar al usuario que ingrese un número entero
    printf("%s\n", message);
    
    // Intenta leer un número entero
    if (scanf("%d", &num) == 1) {
        return num;
    } else {
        printf("Entrada inválida. Por favor, intentar de nuevo.\n");
        // Limpia el búfer de entrada
        while (getchar() != '\n'); // Leer y descartar caracteres hasta encontrar un salto de línea
    }
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

	send(client_fd, buffer, buffer_size, 0);
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


	// Esperar la finalización de hilo de espera
    pthread_join(thread_id, NULL);

	// closing the connected socket
	close(client_fd);
	return 0;
}
