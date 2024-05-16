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

