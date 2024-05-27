#include "chat.pb-c.h"
#include <stdlib.h>


struct Buffer {
    uint8_t *buffer;
    size_t buffer_size;
};

struct Buffer get_register_user_request(char *username){
    Chat__Request request = CHAT__REQUEST__INIT;
	request.operation = CHAT__OPERATION__REGISTER_USER;

	Chat__NewUserRequest new_user_req = CHAT__NEW_USER_REQUEST__INIT;
	new_user_req.username = username;

	request.register_user = &new_user_req;
	request.payload_case = CHAT__REQUEST__PAYLOAD_REGISTER_USER;
    
    size_t buffer_size = chat__request__get_packed_size(&request);

    // Asignar memoria para el buffer
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);

    // Serializar la estructura de mensaje en el buffer
    chat__request__pack(&request, buffer);

    struct Buffer request_buf = {
        buffer,
        buffer_size
    };
    return request_buf;
}

struct Buffer get_unregister_user_request(char *username){
    Chat__Request request = CHAT__REQUEST__INIT;
	request.operation = CHAT__OPERATION__UNREGISTER_USER;

	Chat__User user = CHAT__USER__INIT;
	user.username = username;

	request.unregister_user = &user;
	request.payload_case = CHAT__REQUEST__PAYLOAD_UNREGISTER_USER;
    
    size_t buffer_size = chat__request__get_packed_size(&request);

    // Asignar memoria para el buffer
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);

    // Serializar la estructura de mensaje en el buffer
    chat__request__pack(&request, buffer);

    struct Buffer request_buf = {
        buffer,
        buffer_size
    };
    return request_buf;
}


struct Buffer get_user_list_request(char *username){
    Chat__Request request = CHAT__REQUEST__INIT;
	request.operation = CHAT__OPERATION__GET_USERS;

    if(username != NULL){
        Chat__UserListRequest user_list_req = CHAT__USER_LIST_REQUEST__INIT;
        user_list_req.username = username;
        request.get_users = &user_list_req;
    }
	request.payload_case = CHAT__REQUEST__PAYLOAD_GET_USERS;
    
    size_t buffer_size = chat__request__get_packed_size(&request);

    // Asignar memoria para el buffer
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);

    // Serializar la estructura de mensaje en el buffer
    chat__request__pack(&request, buffer);

    struct Buffer request_buf = {
        buffer,
        buffer_size
    };
    return request_buf;
}

char* get_user_status(int status){
    if(status == 0){
        return GREEN "ONLINE" RESET;
    }else if(status == 1){
        return YELLOW "BUSY" RESET;
    }else{
        return RED "OFFLINE" RESET;
    }
}

struct Buffer get_update_status_request(int new_status, char* username){
    Chat__Request request = CHAT__REQUEST__INIT;
	request.operation = CHAT__OPERATION__UPDATE_STATUS;

    Chat__UpdateStatusRequest update_status_req = CHAT__UPDATE_STATUS_REQUEST__INIT;
    update_status_req.new_status = new_status;
    update_status_req.username = username;
	request.payload_case = CHAT__REQUEST__PAYLOAD_UPDATE_STATUS;
    request.update_status = &update_status_req;

    size_t buffer_size = chat__request__get_packed_size(&request);

    // Asignar memoria para el buffer
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);

    // Serializar la estructura de mensaje en el buffer
    chat__request__pack(&request, buffer);

    struct Buffer request_buf = {
        buffer,
        buffer_size
    };
    return request_buf;
}

/**
 * Función que genera el buffer para la solicitud de enviar mensaje.
 * @param recipient_username char*. (opcional) Nombre del usuario destino. Si es null, el mensaje se envía a todos.
 * @param message char*. Mensaje a enviar.
*/
struct Buffer get_send_message_request(char* recipient_username, char* message){
    Chat__Request request = CHAT__REQUEST__INIT;
	request.operation = CHAT__OPERATION__SEND_MESSAGE;

    Chat__SendMessageRequest send_message_req = CHAT__SEND_MESSAGE_REQUEST__INIT;
    send_message_req.content = message;
    if(recipient_username) send_message_req.recipient = recipient_username;

    request.send_message = &send_message_req;
    request.payload_case = CHAT__REQUEST__PAYLOAD_SEND_MESSAGE;


    size_t buffer_size = chat__request__get_packed_size(&request);

    // Asignar memoria para el buffer
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);

    // Serializar la estructura de mensaje en el buffer
    chat__request__pack(&request, buffer);

    struct Buffer request_buf = {
        buffer,
        buffer_size
    };
    return request_buf;
}