#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include <stddef.h>
#include "chat.pb-c.h"
#include <stdlib.h>

struct User {
    int connection_fd;
    char* name;
    char* ip;
    int status;
    struct User* next_user;
};

struct User* first_user = NULL;
struct User* last_user = NULL;

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

    return NULL;
}

void print_usernames(){
    
    struct User *curr_user = first_user;
    while(curr_user != NULL){
        printf("%s\n", curr_user->name);
        curr_user = curr_user->next_user;
    }
}

Chat__Response get_response_object(int operation, int status_code, char *message){
    Chat__Response response = CHAT__RESPONSE__INIT;
    response.operation = operation;
    response.status_code = status_code;
    response.message = message;

    return response;
}

#endif