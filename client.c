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
#include "client_functions.h"
#define PORT 8080

char *username = NULL;
bool provitional_username = true;
bool lock_menu = false;
bool connection_open = true;
bool stop_message_input = false;

void *thread_listening_server(void *param) {

	int socket_id = *((int *)param);
    uint8_t *buffer;

    while (connection_open) {

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

		// Convertir a objeto request
		Chat__Response *response = chat__response__unpack(NULL, bytes_read, buffer);

		// Liberar memoria de buffer
		free(buffer);

		if(response != NULL){

			if(response->message != NULL){
				printf("Respuesta del servidor: (%s) %s\n",response->status_code == CHAT__STATUS_CODE__OK ? "ok" : "error", response->message);
			}

			if(response->operation == CHAT__OPERATION__REGISTER_USER){

				// Respuesta del servidor para registro de usuario
				
				if(response->status_code != CHAT__STATUS_CODE__OK){
					// No se registró el nombre de usuario, resetear
					username = NULL;
					provitional_username = true;
				} else {
					provitional_username = false; // username aceptado
				}

				lock_menu = false; // Liberar bloqueo de menu
			
			}else if(response->operation == CHAT__OPERATION__UNREGISTER_USER){

				// Respuesta del servidor para logout de usuario
				if(response->status_code == CHAT__STATUS_CODE__OK){
					username = NULL;
				}

				lock_menu = false; // Liberar bloqueo de menu

			}else if(response->operation == CHAT__OPERATION__GET_USERS){

				if(response->status_code == CHAT__STATUS_CODE__OK && response->user_list != NULL){
					
					if(response->user_list->type == CHAT__USER_LIST_TYPE__ALL){

						// Respuesta del servidor con "lista" de usuarios
						printf("Lista de usuarios conectados:\n");
						for(int i = 0; i < response->user_list->n_users; i++){
							Chat__User *user = response->user_list->users[i];
							printf("%d. (%s) %s - %s\n", i + 1, user->ip_address, user->username, get_user_status(user->status));
						}
					}else if(response->user_list->n_users > 0){
						// Datos de un usuario
						Chat__User *user = response->user_list->users[0];
						printf("Datos de usuario:\n- username: %s\n- ip: %s\n- status: %s\n", user->username, user->ip_address, get_user_status(user->status));
					}

				}

				lock_menu = false; // Liberar bloqueo de menu
			}else if(response->operation == CHAT__OPERATION__UPDATE_STATUS){
				// Respuesta del servidor al actualizar status
				lock_menu = false;
			}else if (response->operation == CHAT__OPERATION__SEND_MESSAGE){

				// Respuesta del servidor al enviar mensaje (solo lo recibe el emisor)
				if(response->status_code != CHAT__STATUS_CODE__OK){
					// Si hay error al enviar mensaje, parar el ciclo para continuar enviando
					stop_message_input = true;
				}
			
			}else if (response->operation == CHAT__OPERATION__INCOMING_MESSAGE){

				// Mensaje recibido
				if(response->incoming_message){
					printf("Mensaje de %s (%s): %s\n",
					 	response->incoming_message->sender,
						response->incoming_message->type == CHAT__MESSAGE_TYPE__DIRECT ? "directo" : "todos",
					    response->incoming_message->content);
				}
			}
			
		}else{
			printf("Mensaje recibido del servidor no válido.\n");
		}		

    }

	printf("Conexión con el servidor ha sido cerrada.\n");

    // Cerrar el socket y salir del hilo
	
	connection_open = false;
    close(socket_id);
    pthread_exit(NULL);
}

int read_number(char *message){
	int num;
    
	while(1){
		// Solicitar al usuario que ingrese un número entero
		printf("%s\n", message);
		
		// Intenta leer un número entero
		if (scanf("%d", &num) == 1) {
			while (getchar() != '\n'); // Limpiar input
			return num;
		} else {
			printf("Entrada inválida. Por favor, intentar de nuevo.\n");
			// Limpia el búfer de entrada
			while (getchar() != '\n'); // Limpiar input
		}
	}
}

char* read_string(char *message, size_t max_size) {
    char *string = (char*)malloc(max_size * sizeof(char)); 

    // Solicitar al usuario que ingrese una cadena de caracteres
    printf("%s\n", message);

    // Obtener input
    fgets(string, max_size, stdin);

    // Eliminar el carácter de nueva línea al final de la cadena
    size_t len = strlen(string);
    if (len > 0 && string[len - 1] == '\n') {
        string[len - 1] = '\0';
    }

    return string;
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

	
   	// Crear hilo para espera de mensajes continua
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_listening_server, (void *)&client_fd);

	connection_open = true;
	while(connection_open){

		if(lock_menu) continue;

		// Mostrar menú de opciones
		if(username == NULL || provitional_username){

			int option = read_number("##### Menú de opciones #####\n1. Registrar nombre de usuario\n2. Salir\nElegir una opción: ");
			
			if(username != NULL || !provitional_username) continue; // Evitar cambios durante input

			if(option == 1){

				username = read_string("Ingresar username: ", 100);
				provitional_username = true;

				struct Buffer request = get_register_user_request(username);
				send(client_fd, request.buffer, request.buffer_size, 0);
				free(request.buffer);
				printf("Solicitud de registro de username enviada!\n");
				lock_menu = true;
			} else if (option == 2){
				connection_open = false;
			}
			
		}else{
			int option = read_number("##### Menú de opciones #####\n1. Listado de usuarios conectados.\n2. Obtener datos de un usuario conectado.\n3. Actualizar status.\n4. Enviar mensaje broadcast.\n5. Enviar mensaje directo.\n7.Logout de usuario\n8. Salir\nElegir una opción: ");

			if(username == NULL || provitional_username) continue; // Evitar cambios durante input

			if (option == 1){

				// Obtener listado completo de usuarios
				struct Buffer request = get_user_list_request(NULL);
				send(client_fd, request.buffer, request.buffer_size, 0);
				free(request.buffer);
				printf("Solicitud de listado de usuarios enviada!\n");
				lock_menu = true;

			}else if(option == 2){

				// Obtener datos de un usuario
				char *user = read_string("Ingresar nombre del usuario a consultar:", 100);
				struct Buffer request = get_user_list_request(user);
				send(client_fd, request.buffer, request.buffer_size, 0);
				free(request.buffer);
				printf("Solicitud de datos de usuario enviada!\n");
				lock_menu = true;

			}else if(option == 3){

				// Actualizar status de usuario

				// Obtener nuevo status
				int new_status = -1;
				while(new_status == -1){

					new_status = read_number("Seleccionar nuevo status:\n1. ONLINE\n2. BUSY\n3. OFFLINE") - 1;
					if(new_status < 0 || new_status > 2 ){
						printf("Por favor, selecciona una de las opciones.\n");
						new_status = -1;
					}
				}

				struct Buffer request = get_update_status_request(new_status, username);
				send(client_fd, request.buffer, request.buffer_size, 0);
				free(request.buffer);
				printf("Solicitud de cambio de status enviada!\n");
				lock_menu = true;
			}else if (option == 4){

				// Enviar mensaje a todos los usuarios

				// Si ocurre un error con un envío, se para el input
				while(!stop_message_input){
					char *message = read_string("Enviar mensaje a todos los usuarios conectados (:q para salir):", 500);

					if(stop_message_input || strcmp(message, ":q") == 0) break;

					struct Buffer request = get_send_message_request(NULL, message);
					send(client_fd, request.buffer, request.buffer_size, 0);
					free(request.buffer);

				}
				stop_message_input = false;
			}
			else if (option == 5){

				// Enviar mensaje a todos los usuarios
				char *recipient = read_string("Nombre del usuario para chatear:", 100);
				char prompt[600]; // Concatenar mensaje y nombre
				sprintf(prompt, "Enviar mensaje directo a %s (:q para salir):", recipient);

				// Stop direct message puede indicar que el usuario proporcionado no existe
				while(!stop_message_input){
					
					char *message = read_string(prompt, 500);

					if(stop_message_input || strcmp(message, ":q") == 0) break;


					struct Buffer request = get_send_message_request(recipient, message);
					send(client_fd, request.buffer, request.buffer_size, 0);
					free(request.buffer);
					free(message);
				}

				free(recipient);
				stop_message_input = false;

			}else if(option == 7){

				// Logout de usuario
				struct Buffer request = get_unregister_user_request(username);
				send(client_fd, request.buffer, request.buffer_size, 0);
				free(request.buffer);
				printf("Solicitud de logout enviada!\n");
				lock_menu = true;
			}else if(option == 8){
				connection_open = false;
			}
		}
		
	}

	printf("Cerrando conexión...\n");
	// Cerrar conexión
	close(client_fd);

	return 0;
}
