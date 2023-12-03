#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  char buffer[BUFFER_SIZE];

  // 서버 소켓 생성
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 서버 주소 설정
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // 서버 소켓 바인딩
  if (bind(server_socket, (struct sockaddr*)&server_addr,
           sizeof(server_addr)) == -1) {
    perror("Binding failed");
    exit(EXIT_FAILURE);
  }

  // 클라이언트로부터의 연결 요청 대기
  if (listen(server_socket, 5) == -1) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", PORT);

  // 클라이언트와 연결
  if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr,
                              &client_addr_len)) == -1) {
    perror("Accept failed");
    exit(EXIT_FAILURE);
  }

  printf("Client connected\n");

  // 데이터 수신 및 송신
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socket, buffer, sizeof(buffer), 0) == -1) {
      perror("Receive failed");
      exit(EXIT_FAILURE);
    }

    if (strcmp(buffer, "exit") == 0) {
      printf("Client disconnected\n");
      break;
    }

    printf("Received message: %s\n", buffer);

    if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
      perror("Send failed");
      exit(EXIT_FAILURE);
    }
  }

  // 소켓 종료
  close(client_socket);
  close(server_socket);

  return 0;
}
