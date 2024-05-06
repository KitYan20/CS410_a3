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
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>



//#include <windows.h>

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

void execute_script(int client_socket,const char *path,char qargs[][BUFFER_SIZE],int length){
    char script_path[BUFFER_SIZE];
    FILE *script_file = fopen(path,"rb");
    if (script_file == NULL){
        send_response(client_socket, "404 Not Found", "text/plain", "File not found");
        return;
    }
    char shebang[BUFFER_SIZE];
    //Gets the shebang of the cgi file to determine the interpreter
    fgets(shebang,sizeof(shebang),script_file);
    fclose(script_file);
    char interpreter[BUFFER_SIZE];
    if (strstr(shebang,"#!/usr/bin/python") != NULL){
        strcpy(interpreter,"/usr/bin/python3");
    }else if (strstr(shebang,"#!/bin/sh") != NULL){
        strcpy(interpreter,"/bin/sh");
    }else{ 
        send_response(client_socket, "500 Internal Server Error", "text/plain", "Failed to read script");
        return; 
    }

    int pipefd[2];
    
    if(pipe(pipefd) == -1){
        perror("pipe");
        send_response(client_socket,"500 Internal Server Error","text/plain","Failed to create pipe");
        return;
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
        execl(interpreter, interpreter, path,qargs[1], NULL);
        perror("execl");
        exit(1);
    }else{
        // Parent process
        close(pipefd[1]);
        char output[BUFFER_SIZE] = "";
        char buffer[BUFFER_SIZE];
        long long bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            strncat(output, buffer, bytes_read);
        }
        close(pipefd[0]);
        wait(NULL);
        if (strcmp(interpreter,"/usr/bin/python3") == 0){
            send_response(client_socket, "200 OK", "text/html", output);
        }else{
            send_response(client_socket, "200 OK", "text/plain", output);
        }
    }
}

void send_file_content(int client_socket, const char *content_type, long file_size, FILE *file) {
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             content_type, file_size);
    send(client_socket, header, strlen(header), 0);

    char buffer[BUFFER_SIZE];
    long long bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
}

void serve_file(int client_socket, const char *path,char *query) {

    char *string= strtok(query,"&");
    char *token = strtok(string,"=");
    char qargs[BUFFER_SIZE][BUFFER_SIZE];
    int i = 0;
    while( token != NULL ) {
        strcpy(qargs[i], token);  // copy token to the string array
        i++;
        token = strtok(NULL, " ");      
   }
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
        } else if (strcmp(file_extension,".cgi") == 0){
            execute_script(client_socket,path,qargs,i);
            return;
        }
    }

    send_file_content(client_socket, content_type, file_stat.st_size, file);
    fclose(file);
}

void handle_arduino_request(int client_socket) {
  // Send an initial response to the client with the start button
  const char *initial_response = "<html><body>"
                                 "<h1>Welcome to the Game!</h1>"
                                 "<p>Click the button below to start the game.</p>"
                                 "<form action='/start' method='POST'>"
                                 "<input type='submit' value='Start Game'>"
                                 "</form>"
                                 "</body></html>";
  send_response(client_socket, "200 OK", "text/html", initial_response);

  // Wait for the start button to be clicked
  FILE *fpin;
  char request[1024];
  fpin = fdopen(client_socket, "r");
  fgets(request, 1024, fpin);
  printf("Got a call: request = %s\n", request);

  // Check if the start button was clicked
  if (strstr(request, "POST /start") != NULL) {
    // Send the game started response
const char *game_started_response = "<html>\n"
"<head></head>\n"
"<body>\n"
"    <h1>Game Started</h1>\n"
"    <p>Time remaining: <span id=\"timer\">60</span> seconds</p>\n"
"    <p>Score: <span id=\"score\"></span></p>\n"
"    <script>\n"
"        var timeLeft = 60;\n"
"        var timer = setInterval(function() {\n"
"            timeLeft--;\n"
"            document.getElementById('timer').textContent = timeLeft;\n"
"            if (timeLeft <= 0) {\n"
"                clearInterval(timer);\n"
"                document.getElementById('timer').textContent = 'Time\\'s up!';\n"
"                updateScore();\n"
"            }\n"
"        }, 1000);\n"
"        function updateScore() {\n"
"            fetch('/get-score')\n"
"                .then(response => response.text())\n"
"                .then(score => {\n"
"                    document.getElementById('score').textContent = score;\n"
"                });\n"
"        }\n"
"    </script>\n"
"</body>\n"
"</html>";
    send_response(client_socket, "200 OK", "text/html", game_started_response);

    // Define the Python script
    const char *python_script = "import serial\n"
                                "import time\n"
                                "\n"
                                "arduino = serial.Serial('/dev/ttyACM0', 9600)\n"
                                "time.sleep(2)  # Wait for the Arduino to initialize\n"
                                "\n"
                                "arduino.write(b'start\\n')  # Send the 'start' message\n"
                                "\n"
                                "start_time = time.time()\n"
                                "score = arduino.readline().decode('utf-8').strip()\n"
                                "while (time.time() - start_time) < 60:\n"
                                "    if arduino.in_waiting > 0:\n"
                                "        score = arduino.readline().decode('utf-8').strip()  # Read the score\n"
                                "print(score)\n"
                                "\n"
                                "arduino.close()\n";

    // Create a temporary file to store the Python script
    FILE *temp_file = fopen("temp_script.py", "w");
    if (temp_file == NULL) {
      printf("Error creating temporary file\n");
      return;
    }

    // Write the Python script to the temporary file
    fprintf(temp_file, "%s", python_script);
    fclose(temp_file);

    // Execute the Python script and capture the output
    char command[256];
    sprintf(command, "python3 temp_script.py");
    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
      printf("Error executing Python script\n");
      remove("temp_script.py");
      return;
    }

// Read the score from the Python script output
char score_str[16];
if (fgets(score_str, sizeof(score_str), pipe) != NULL) {
  int score = atoi(score_str);
  printf("Received score from Arduino: %d\n", score);

  // Update the HTML with the score
  char score_html[256];
  sprintf(score_html, "%d", score);
  send_response(client_socket, "200 OK", "text/html", score_html);
} else {
      printf("Error reading score from Python script output\n");
    }

    // Close the pipe and remove the temporary file
    pclose(pipe);
    remove("temp_script.py");
  }

  fclose(fpin);
  close(client_socket);
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
    char *arguments = strtok(path, "?");
    char *query = strtok(NULL,"");

    if (path != NULL && strcmp(method,"GET") == 0){
        if (strcmp(path, "/") == 0 || path[strlen(path) - 1] == '/') {
                directory_listing(client_socket, ".");
        }else if (strcmp(path,"/start") == 0){
            handle_arduino_request(client_socket);
        }else{
            serve_file(client_socket, path + 1,query);
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