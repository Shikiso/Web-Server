#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080
#define file_read_size 100

char *read_content(char *file){
  FILE *fptr;
  int character;
  int position = 0;
  size_t content_size = file_read_size;
  char *content = malloc(file_read_size);

  char file_name[50] = ".";
  strcat(file_name, file);

  fptr = fopen(file_name, "r");
  if (fptr == NULL){
    return "NULL";
  }

  while ((character = fgetc(fptr)) != EOF){
    content[position] = character;
    position++;

    if (position == sizeof(content)){
      content_size += file_read_size;
      content = realloc(content, content_size);
    }
  }

  fclose(fptr);
  return content;
}

char *get_page(char *request){
  char *line = malloc(50);
  int position = 0;
  char *page = malloc(50);
  int capture_page = 0;
  
  // Get first line of request
  for (int i=0; i<strlen(request); i++){
    if (request[i] != '\n'){
      line[position] = request[i];
      position++;
    }
    else{
      break;
    }

    if (position == sizeof(line)){
      line = realloc(line, sizeof(line)+50);
    }
  }
  
  // Get page
  position = 0;
  for (int i=0; i<strlen(line); i++){
    if (line[i] != ' '){
      if (capture_page == 1){
        page[position] = line[i];
        position++;

        if (position == sizeof(page)){
          page = realloc(page, sizeof(page)+50);
        }
      }
    }
    else{
      if (capture_page == 0){
        capture_page = 1;
      }
      else{
        page[position] = '\0';
        break;
      }
    }
  }
  return page;
}

void handle_request(int client_socket, char *request){
  char *page = get_page(request);
  if (!strcmp(page, "/")){
    strcpy(page, "/index.html");
  }
  char *content = read_content(page);
  char header[300];
  char end[] = "\r\n";
  char *response;

  printf("Getting %s\n", page);
  if (!strcmp(content, "NULL")){
    printf("Not Found!\n");
    strcpy(header, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\nFile Not Found\r\n");
    send(client_socket, header, strlen(header), 0);
  }
  else{
    printf("Found serving...\n");
    strcpy(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    response = malloc(strlen(header) + strlen(content) + strlen(end) + 1);
    strcpy(response, header);
    strcat(response, content);
    strcat(response, end);
    send(client_socket, response, strlen(response), 0);
  }
  
}

int main(int argc, char *argv[]){
  int sock, client_socket;
  struct sockaddr_in address;
  socklen_t addrlen = sizeof(address);
  char *content;
  char request[600];

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1){
    perror("Socket failure");
    return 1;
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(sock, (struct sockaddr*)&address, sizeof(address)) == -1){
    perror("Bind failure");
    return 1;
  }

  if (listen(sock, 3) == -1){
    perror("Listen failure");
    return 1;
  }
  
  printf("Listening on port %d\n", PORT);
  while (1){
    if ((client_socket = accept(sock, (struct sockaddr*)&address, &addrlen)) == -1){
      perror("Accept failure");
      exit(EXIT_FAILURE);
    };
  
    printf("Connection Initialized!\n");
    recv(client_socket, request, sizeof(request), 0);
    handle_request(client_socket, request);

    close(client_socket);
  }
  close(sock);
  return 0;
}
