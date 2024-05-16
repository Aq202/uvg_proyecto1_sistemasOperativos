#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <asm-generic/socket.h>
#include <pthread.h> 
#include <arpa/inet.h>
#include <stdbool.h>
#include "chat.pb-c.h"
#include "server_functions.h"
#include "consts.h"
#define PORT 8080


struct Connection {
	int fd;
	char *ip;
};

void *thread_listening_client(void *param) {
	struct Connection cn = *((struct Connection *)param);
    int socket_id = cn.fd;
	char *ip = cn.ip;
    printf("Id del socket del cliente: %d, ip: %s\n", socket_id, ip);

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
		Chat__Request *request = chat__request__unpack(NULL, bytes_read, buffer);

		// Liberar memoria de buffer
		free(buffer);

		if(request != NULL){

			printf("Request type: %d\n", request->operation);

			if(request->operation == CHAT__OPERATION__REGISTER_USER){

				// Registrar usuario

				char* result = register_user(socket_id, request->register_user->username, ip);
				char* message = "Usuario registrado exitosamente!";
				int status = 200;
				if(result != NULL){
					// Hay error, enviar mensaje y status de error
					message = result;
					status = 400;
				}

				// Enviar respuesta
				struct Buffer res_buff = get_simple_response(CHAT__OPERATION__REGISTER_USER, status, message);
				
				send(socket_id, res_buff.buffer, res_buff.buffer_size, 0);
				free(res_buff.buffer);
				printf("Respuesta '%s' enviada.\n", message);

			} else if (request->operation == CHAT__OPERATION__UNREGISTER_USER){

				// Desloguear usuario. Conexión permanece abierta.
				char* result = remove_user(socket_id, true);
				char* message = "Logout realizado exitosamente!";
				int status = CHAT__STATUS_CODE__OK;
				if(result != NULL){
					// Hay error, enviar mensaje y status de error
					message = result;
					status = CHAT__STATUS_CODE__BAD_REQUEST;
				}

				// Enviar respuesta
				struct Buffer res_buff = get_simple_response(CHAT__OPERATION__REGISTER_USER, status, message);
				
				send(socket_id, res_buff.buffer, res_buff.buffer_size, 0);
				free(res_buff.buffer);
			}
		}

    }

	// Eliminar al usuario del registro
	remove_user(socket_id, false);
	print_usernames();

	printf("Conexión %d ha sido cerrada.\n", socket_id);

    // Cerrar el socket y salir del hilo
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

		char *client_ip = inet_ntoa(address.sin_addr);

		struct Connection cn = {
			new_socket,
			client_ip
		};

        // Crear thread para conexión aceptada
        pthread_create(&thread_id, NULL, thread_listening_client, (void *)&cn);

        
    }

    pthread_join(thread_id, NULL);
	// closing the listening socket
	close(server_fd);
	return 0;
}
