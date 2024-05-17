#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include <stddef.h>
#include "chat.pb-c.h"
#include <stdlib.h>


struct Buffer {
    uint8_t *buffer;
    size_t buffer_size;
};

struct User {
    int connection_fd;
    char* name;
    char* ip;
    int status;
    struct User* next_user;
};

struct User* first_user = NULL;
struct User* last_user = NULL;
int total_users = 0;

/**
 * Función para obtener un usuario registrado en el servidor. 
 * Se busca por name o ip o ambos.
 * @param name char*. Nombre del usuario.
 * @param ip char*. ip del usuario.
 * @return struct user. Null si no se encuentra el usuario
*/
struct User* get_user(char*name, char*ip){

    struct User* user = first_user;
    while(user != NULL){

        if((name == NULL || strcmp(user->name, name) == 0) && (ip == NULL || strcmp(user->ip, ip) == 0)){
            return user;
        }
        user = user->next_user;
    }
    return NULL;
}

/**
 * Función para registrar un nuevo usuario en el servidor.
 * @param connection_fd int. fd de la conexión por sockets.
 * @param name char*. Nombre del usuario.
 * @param ip char*. ip del usuario.
 * @return String. Error al registrar usuario. NUll si no hay error.
*/
char* register_user(int connection_fd, char* name, char* ip){

    // Verificar que el nombre sea único
    if(get_user(name, NULL) != NULL){
        return "El nombre de usuario seleccionado no esta disponible.";
    }

    struct User* new_user = malloc(sizeof(struct User));
    new_user->connection_fd = connection_fd;
    new_user->name = name;
    new_user->ip = ip;
    new_user->status = CHAT__USER_STATUS__ONLINE;
    new_user->next_user = NULL;

    if(first_user == NULL){
        // Agregar inicio de lista enlazada
        first_user = new_user;
    }

    if(last_user != NULL){
        // Encadenar al final
        last_user->next_user = new_user;
    }

    last_user = new_user;
    total_users += 1;
    return NULL;
}

void print_usernames(){
    
    struct User *curr_user = first_user;
    while(curr_user != NULL){
        printf("%s\n", curr_user->name);
        curr_user = curr_user->next_user;
    }
}

struct Buffer get_simple_response(int operation, int status_code, char *message){
    Chat__Response response = CHAT__RESPONSE__INIT;
    response.operation = operation;
    response.status_code = status_code;
    response.message = message;

    size_t size = chat__response__get_packed_size(&response);
	uint8_t *buffer = (uint8_t *)malloc(size);
		
	chat__response__pack(&response, buffer);

    struct Buffer response_buf = {
        buffer,
        size
    };
    return response_buf;

}

/**
 * Función para remover un usuario del servidor.
 * @param connection_fd int. fd de la conexión por sockets.
 * @param strict bool. Si su valor es true, devuelve un error si el usuario no fue encontrado para luego ser removido.
 * @return String. Error al remover usuario. NUll si no hay error.
*/
char* remove_user(int connection_fd, bool strict){

    if(first_user == NULL && strict){
        return "El usuario no está registrado";
    }

    struct User* user = first_user;
    struct User *user_to_remove = NULL;
    while(user != NULL){

        if(user->connection_fd == connection_fd){

            // Si el usuario actual es el objetivo
             user_to_remove = user;
            
        }else if(user->next_user != NULL && user->next_user->connection_fd == connection_fd){
            // Si el usuario siguiente es el objetivo
            user_to_remove = user->next_user;
            user->next_user = user_to_remove->next_user; // Enlazar al siguiente usuario en la lista
        }

        if(user_to_remove != NULL){
            
            total_users -= 1;

            //ELiminar punteros de inicio y fin de la lista si es un usuario extremo
            if(user_to_remove == first_user){
                first_user = user_to_remove->next_user; 
            }else if (user_to_remove == last_user){
                last_user = NULL;
            }
            free(user_to_remove);
            return NULL;
        }
        user = user->next_user;
    }

    return strict ? "El usuario no esta registrado.": NULL;
}

struct Buffer get_user_list_response(){
    Chat__Response response = CHAT__RESPONSE__INIT;
    response.operation = CHAT__OPERATION__GET_USERS;
    response.status_code = CHAT__STATUS_CODE__OK;
    response.message = "Lista de usuarios conectados enviada correctamente!";

    // Generar lista de usuarios a enviar
    struct Chat__User** users = malloc(total_users * sizeof(struct Chat__User*));
    struct User* user = first_user;
    int users_num = 0;
    while(user != NULL){

        // Guardar puntero de objeto users (protocolo) en lista
        users[users_num] = malloc(sizeof(Chat__User*));

        Chat__User *proto_user = malloc(sizeof(Chat__User));
        chat__user__init(proto_user); // Inicializar el usuario
        proto_user->username = user->name;
        proto_user->ip_address = user->ip;
        proto_user->status = user->status;

        users[users_num] = proto_user;

        user = user->next_user;
        users_num += 1;
    }

    // Objeto que se envía como parte de la response (size, lista de usuarios(punteros) y tipo (all))
    Chat__UserListResponse proto_user_list = CHAT__USER_LIST_RESPONSE__INIT;
    proto_user_list.n_users = (size_t) users_num;
    proto_user_list.users = users;
    proto_user_list.type = CHAT__USER_LIST_TYPE__ALL;


    response.result_case = CHAT__RESPONSE__RESULT_USER_LIST;
    response.user_list = &proto_user_list;

    size_t size = chat__response__get_packed_size(&response);
	uint8_t *buffer = (uint8_t *)malloc(size);
	
	chat__response__pack(&response, buffer);

    // Liberar memoria
    for(int i = 0; i < users_num; i++){
        free(users[i]);
    }
    free(users);

    struct Buffer response_buf = {
        buffer,
        size
    };
    return response_buf;
}

#endif