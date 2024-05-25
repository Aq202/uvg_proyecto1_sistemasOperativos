# Proyecto 1 - Chat en C

El proyecto consiste en un chat desarrollado en C, el cual hace uso de sockets para establecer
comunicación con múltiples clientes en tiempo real.

## Instrucciones de compilación de protocolo

Para compilar la versión en C del protocolo, se debe seguir el procedimiento descrito en el siguiente repositorio para instalar el plugin protobuf-c.

https://github.com/protobuf-c/protobuf-c

Una vez instalado el plugin, el protocolo se compila utilizando el siguiente comando:

```
protoc --c_out=. example.proto
```

## Instrucciones de compilación de servidor y cliente

Ejecutar los siguientes comando y añadir el resultado de ambos a las flags de compilación tanto del server como del client.

```
pkg-config --cflags 'libprotobuf-c >= 1.0.0'
```

```
pkg-config --libs 'libprotobuf-c >= 1.0.0'
```

Por ejemplo, el resultado del primer comando fue **-I/usr/local/include -L/usr/local/lib** y del segundo **-lprotobuf-c**.

Además, se debe compilar el código fuente en C junto con el archivo **.pb-c.c**.

El comando de compilación para el servidor y el cliente podrían verse similar a:

* Compilación del servidor

  ```
  gcc -o server ./server.c chat.pb-c.c -I/usr/local/include -L/usr/local/lib -lprotobuf-c
  ```

* Compilación del cliente

  ```
  gcc -o client ./client.c chat.pb-c.c -I/usr/local/include -L/usr/local/lib -lprotobuf-c
  ```


## Instrucciones de ejecución

Una vez compilados los archivos **server.c** y **client.c**, se deben ejecutar de la siguiente manera:

* Ejecución de servidor

  ```
  ./server <PUERTO>
  ```

* Ejecución de cliente

  ```
  ./server <IP SERVIDOR> <PUERTO>
  ```