#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024


void send_response(int client_socket, const char *status, const char *content_type, const char *content) {

    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %lu\r\n"
             "\r\n"
             "%s",
             status, content_type, strlen(content), content);
    send(client_socket, response, strlen(response), 0);
}

void directory_listing(int client_socket, const char *path) {
    int pipefd[2];
    if(pipe(pipefd) == -1){
        perror("pipe");
        send_response(client_socket,"500 Internal Server Error","text/plain","Failed to create pipe");
    }

    pid_t pid = fork();
    if (pid == -1){
        perror("fork");
        send_response(client_socket, "500 Internal Server Error", "text/plain", "Failed to fork process");
        return;
    }else if(pid == 0){
        close(pipefd[0]);
        dup2(pipefd[1],STDOUT_FILENO);
        close(pipefd[1]);
        execl("/bin/ls","ls", "-l",path,NULL);
        perror("execl");
        exit(1);
    }else{
        // Parent process
        close(pipefd[1]);

        char listing[BUFFER_SIZE] = "";
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            strncat(listing, buffer, bytes_read);
        }
        close(pipefd[0]);
        wait(NULL);

        send_response(client_socket, "200 OK", "text/plain", listing);
    }


}

void serve_file(int client_socket, const char *path) {
    FILE *file = fopen(path, "rb");
    //Will check if file exist
    if (file == NULL) {
        send_response(client_socket, "404 Not Found", "text/plain", "File not found");
        return;
    }

    struct stat file_stat;
    if (stat(path, &file_stat) < 0) {
        fclose(file);
        send_response(client_socket, "500 Internal Server Error", "text/plain", "Failed to get file information");
        return;
    }

    char *file_extension = strrchr(path, '.');
    const char *content_type = "application/octet-stream";
    if (file_extension != NULL) {
        if (strcmp(file_extension, ".html") == 0) {
            content_type = "text/html";
        } else if (strcmp(file_extension, ".jpg") == 0 || strcmp(file_extension, ".jpeg") == 0) {
            content_type = "image/jpeg";
        } else if (strcmp(file_extension, ".gif") == 0) {
            content_type = "image/gif";
        }
    }
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             content_type, file_stat.st_size);
    send(client_socket, header, strlen(header), 0);

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(file);
}

void handle_request(int client_socket){
    FILE *fpin;
    char request[1024];

    fpin = fdopen(client_socket,"r");
    fgets(request,1024,fpin);
    printf("Got a call: request = %s\n",request);

    // Parse the request and extract the path
    char *method = strtok(request, " ");
    char *path = strtok(NULL, " ");

    if (path != NULL && strcmp(method,"GET") == 0){
        if (strcmp(path, "/") == 0 || path[strlen(path) - 1] == '/') {
                directory_listing(client_socket, ".");
        } else {
                serve_file(client_socket, path + 1);
        }
    }else{
        send_response(client_socket, "400 Bad Request", "text/plain", "Bad Request");
        
    }

    fclose(fpin);
    close(client_socket);

}
int main(int argc, char* argv[]){
    int PORT_NUMBER = atoi(argv[1]);

    int server_socket, client_socket;
    struct sockaddr_in server_address,client_address;
    int address_length = sizeof(client_address);

        //Create a server socket SOCK_STREAM for socket, 0 for protocol 0
    if ((server_socket = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("(servConn): socket() error");
        exit (-1);
    }
    //Be able to reuse the socket PORT number
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
        // Parse the request and extract the path

        handle_request(client_socket);

        
    }
    close(server_socket);
    return 0;
}