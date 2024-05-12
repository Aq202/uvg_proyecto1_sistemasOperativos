#include <stdio.h>
#include <stdlib.h>
#include "chat.pb-c.h"


int main() {
    // Crear una nueva instancia de la estructura de mensaje para el usuario
    Chat__User user = CHAT__USER__INIT;
    user.username = "Diego";
    user.ip_address = "192.165.254.25adfasfasdf adsfasdf asdf asdfas dfasdf asdfasfa";
    user.status = CHAT__USER_STATUS__ONLINE;
    
    size_t buffer_size = chat__user__get_packed_size(&user);

    // Asignar memoria para el buffer
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);

    // Serializar la estructura de mensaje en el buffer
    chat__user__pack(&user, buffer);

        // Imprimir el contenido del buffer (solo para fines de demostración)
    printf("Buffer serializado: ");
    for (size_t i = 0; i < buffer_size; i++) {
        printf("%02x", buffer[i]);
    }
    printf("\n");


    // Deserializar el buffer y recuperar la estructura de mensaje
    Chat__User *deserialized_user = chat__user__unpack(NULL, buffer_size, buffer);
    if (deserialized_user == NULL) {
        fprintf(stderr, "Error al deserializar el buffer\n");
        return 1;
    }

    // Ahora puedes acceder a los campos de la estructura deserializada
    printf("Username: %s\n", deserialized_user->username);
    printf("IP Address: %s\n", deserialized_user->ip_address);
    printf("Status: %d\n", deserialized_user->status);

    // Liberar la memoria utilizada por la estructura deserializada
    chat__user__free_unpacked(deserialized_user, NULL);


    // Liberar la memoria utilizada por el buffer
    free(buffer);

    return 0;
}
