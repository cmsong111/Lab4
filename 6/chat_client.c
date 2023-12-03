#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int client_socket;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  // 클라이언트 소켓 생성
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 서버 주소 설정
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  server_addr.sin_port = htons(PORT);

  // 서버에 연결
  if (connect(client_socket, (struct sockaddr*)&server_addr,
              sizeof(server_addr)) == -1) {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  printf("Connected to chat server\n");

  // 데이터 송신 및 수신
  while (1) {
    printf("Enter message (type 'exit' to disconnect): ");
    fgets(buffer, sizeof(buffer), stdin);

    if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
      perror("Send failed");
      exit(EXIT_FAILURE);
    }

    if (strcmp(buffer, "exit\n") == 0) {
      printf("Disconnected from server\n");
      break;
    }

    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socket, buffer, sizeof(buffer), 0) == -1) {
      perror("Receive failed");
      exit(EXIT_FAILURE);
    }

    printf("Server response: %s\n", buffer);
  }

  // 클라이언트 소켓 종료
  close(client_socket);

  return 0;
}
