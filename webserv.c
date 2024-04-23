#include <netinet/in.h> // sockaddr_in
#include <netdb.h> // getnameinfo
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h> // socket APIs
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void handle_websocket(int client_socket){
    const char *message = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";;
    send(client_socket, message, strlen(message), 0);
    close(client_socket);
}

int main(int argc, char* argv[]){
    int PORT_NUMBER = atoi(argv[1]);

    int server_socket, client_socket;
    struct sockaddr_in server_address,client_address;
    int address_length = sizeof(client_address);

        //Create a server socket
    if ((server_socket = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("(servConn): socket() error");
        exit (-1);
    }
    //Be able to reuse socket
    int sock_opt_val = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &sock_opt_val, sizeof(sock_opt_val)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons (PORT_NUMBER);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind server socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("bind");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("listen");
        exit(1);
    }
    printf("WebSocket server started on port %d\n", PORT_NUMBER);

    while(1){
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&address_length);

        if (client_socket == -1) {
            perror("accept");
            exit(1);
        }
        printf("Client connected\n");
        // Handle WebSocket communication
        handle_websocket(client_socket);
    }
    close(server_socket);
    return 0;
}