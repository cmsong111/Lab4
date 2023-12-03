#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];

fd_set read_fds;

void broadcast_message(char* message, int sender_fd, int max_fd) {
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    int client_fd = clients[i];
    if (client_fd > 0 && client_fd != sender_fd) {
      send(client_fd, message, strlen(message), 0);
    }
  }
}

int main() {
  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  char buffer[BUFFER_SIZE];

  FD_ZERO(&read_fds);

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
  if (listen(server_socket, MAX_CLIENTS) == -1) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Chat server listening on port %d...\n", PORT);

  while (1) {
    // 현재 클라이언트 소켓을 모두 설정
    FD_SET(server_socket, &read_fds);
    int max_fd = server_socket;

    for (int i = 0; i < MAX_CLIENTS; ++i) {
      int client_fd = clients[i];
      if (client_fd > 0) {
        FD_SET(client_fd, &read_fds);
        if (client_fd > max_fd) {
          max_fd = client_fd;
        }
      }
    }

    // select 함수를 이용하여 읽기 가능한 소켓을 검사
    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
      perror("Select failed");
      exit(EXIT_FAILURE);
    }

    // 서버 소켓에서 클라이언트의 연결 요청을 처리
    if (FD_ISSET(server_socket, &read_fds)) {
      if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr,
                                  &client_addr_len)) == -1) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
      }

      printf("New client connected\n");

      // 클라이언트를 배열에 추가
      for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] == 0) {
          clients[i] = client_socket;
          break;
        }
      }
    }

    // 클라이언트로부터 메시지를 수신하고 전체에게 방송
    for (int i = 0; i < MAX_CLIENTS; ++i) {
      int client_fd = clients[i];
      if (client_fd > 0 && FD_ISSET(client_fd, &read_fds)) {
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_fd, buffer, sizeof(buffer), 0) <= 0) {
          // 클라이언트 연결 종료
          printf("Client disconnected\n");
          close(client_fd);
          FD_CLR(client_fd, &read_fds);
          clients[i] = 0;
        } else {
          // 받은 메시지를 모든 클라이언트에게 방송
          printf("Received message: %s\n", buffer);
          broadcast_message(buffer, client_fd, max_fd);
        }
      }
    }
  }

  // 서버 소켓 종료
  close(server_socket);

  return 0;
}
