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

			if(response->operation == CHAT__OPERATION__REGISTER_USER){

				
				if(response->status_code != CHAT__STATUS_CODE__OK){
					// No se registró el nombre de usuario, resetear
					username = NULL;
					provitional_username = true;
					printf("Respuesta del servidor: (error) %s\n", response->message);
				} else {
					provitional_username = false; // username aceptado
					printf("Respuesta del servidor: (ok) %s\n", response->message);

				}

				lock_menu = false; // Liberar bloqueo de menu
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
			
			if(option == 1){

				username = read_string("Ingresar username: ", 100);

				struct Buffer request = get_register_user_request(username);
				send(client_fd, request.buffer, request.buffer_size, 0);
				free(request.buffer);
				printf("Solicitud de registro de username enviada!\n");
				lock_menu = true;
			} else if (option == 2){
				connection_open = false;
			}
			
		}else{
			int option = read_number("##### Menú de opciones #####\n1. Salir\nElegir una opción: ");
			
			if(option == 1){
				connection_open = false;
			}
		}
		
	}

	printf("Cerrando conexión...\n");
	// Cerrar conexión
	close(client_fd);

	return 0;
}
