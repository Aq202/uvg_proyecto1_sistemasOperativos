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

struct Connection {
	int fd;
	char *ip;
};

void *thread_listening_client(void *param) {
	struct Connection cn = *((struct Connection *)param);
    int socket_id = cn.fd;
	char *ip = cn.ip;
    printf("Cliente conectado: %d, ip: %s\n", socket_id, ip);

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

		// Buscar objeto de usuario
		struct User *current_user = get_user(NULL, NULL, &socket_id);
		if(current_user != NULL){

			// Actualizar última interacción e intentar actualizar estado a online
			update_user_last_interaction(current_user);
			auto_update_online_user_status(current_user);
		}
		

		// Redimensionar buffer a tamaño de mensaje leído
		buffer = (uint8_t *)realloc(buffer, bytes_read);


		printf("Mensaje recibido de cliente %d %s: %s\n", socket_id, ip, (char*) buffer);

		// Convertir a objeto request
		Chat__Request *request = chat__request__unpack(NULL, bytes_read, buffer);

		// Liberar memoria de buffer
		free(buffer);

		if(request != NULL){

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
				printf("Respuesta '%s' enviada a cliente %d %s.\n", message, socket_id, ip);

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
				struct Buffer res_buff = get_simple_response(CHAT__OPERATION__UNREGISTER_USER, status, message);
				
				send(socket_id, res_buff.buffer, res_buff.buffer_size, 0);
				free(res_buff.buffer);
			
			} else if (request->operation == CHAT__OPERATION__GET_USERS){

				// Obtener lista de usuarios
				struct Buffer res_buff;
				if(request->get_users == NULL || request->get_users->username == NULL || strlen(request->get_users->username) == 0){

					// Obtener lista completa
					res_buff = get_user_list_response(NULL);
				
				}else{
					// Enviar info de un usuario
					printf("NOMBRE DE USUARIO :%s:\n",request->get_users->username);
					res_buff = get_user_list_response(request->get_users->username);
				}
				send(socket_id, res_buff.buffer, res_buff.buffer_size, 0);
				free(res_buff.buffer);
							
			
			}else if (request->operation == CHAT__OPERATION__UPDATE_STATUS){

				char* result; // Error por defecto
				
				// Validar status 
				if(request->update_status == NULL){
					result = result = "Solicitud incompleta.";
				} else if (request->update_status->new_status < 0 || request->update_status->new_status > 2){
					result = result = "El status proporcionado no existe (status validos: 0,1,2).";
				}else{
					result = update_user_status(&socket_id, NULL, request->update_status->new_status, true);
				}

				char* message = "Status del usuario actualizado exitosamente!";
				int status = CHAT__STATUS_CODE__OK;
				if(result != NULL){
					// Hay error, enviar mensaje y status de error
					message = result;
					status = CHAT__STATUS_CODE__BAD_REQUEST;
				}

				// Enviar respuesta
				struct Buffer res_buff = get_simple_response(CHAT__OPERATION__UPDATE_STATUS, status, message);
				
				send(socket_id, res_buff.buffer, res_buff.buffer_size, 0);
				free(res_buff.buffer);
			
			}else if(request->operation == CHAT__OPERATION__SEND_MESSAGE){

				// Operación para enviar mensaje

				char* message = "Mensaje enviado exitosamente!";
				int status = CHAT__STATUS_CODE__OK;
				if(request->send_message == NULL || request->send_message->content == NULL){
					// Hay error, enviar mensaje y status de error
					message = "Solicitud incompleta.";
					status = CHAT__STATUS_CODE__BAD_REQUEST;
				}else{

					if(request->send_message->recipient && strlen(request->send_message->recipient) > 0){
						// Mensaje directo
						struct User *recipient_user = get_user(request->send_message->recipient, NULL, NULL);
						if(!recipient_user){
							message = "El usuario no esta registrado.";
							status = CHAT__STATUS_CODE__BAD_REQUEST;
						}else{
							// ENviar mensaje directo a usuario
							struct Buffer res_message_buff = get_send_message_response(current_user->name, request->send_message->content, CHAT__MESSAGE_TYPE__DIRECT);
							send(recipient_user->connection_fd, res_message_buff.buffer, res_message_buff.buffer_size, 0);
							free(res_message_buff.buffer);
						}
					}else{
						// Mensaje global

						// Iterar usuarios y enviar mensaje
						struct User *recipient_user = get_next_user(NULL);
						while(recipient_user){
							struct Buffer res_message_buff = get_send_message_response(current_user->name, request->send_message->content, CHAT__MESSAGE_TYPE__BROADCAST);
							send(recipient_user->connection_fd, res_message_buff.buffer, res_message_buff.buffer_size, 0);
							free(res_message_buff.buffer);

							recipient_user = get_next_user(recipient_user);
						}
					}
				}
				// Enviar respuesta a emisor
				struct Buffer res_buff = get_simple_response(CHAT__OPERATION__SEND_MESSAGE, status, message);
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
	if (argc != 2) {
        printf("Agregar parametro <puerto>\n");
        return -1;
    }

	int port = atoi(argv[1]); // Obtener puert de parametros
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
	address.sin_port = htons(port);

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

	printf("Servidor escuchando en puerto %d\n",port);

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
