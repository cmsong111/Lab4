// 채팅 서버에서 구현한 서버를 기반으로 간단한 채팅 클라이언트를 구현합니다.
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
  int client_socket;
  struct sockaddr_in server_addr;
  char username[50];
  char message[BUFFER_SIZE];

  // 클라이언트 소켓 생성
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 서버 주소 설정
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 서버 IP 주소
  server_addr.sin_port = htons(8888);                    // 서버 포트 번호

  // 서버에 연결
  if (connect(client_socket, (struct sockaddr*)&server_addr,
              sizeof(server_addr)) == -1) {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  // 사용자 이름 입력
  printf("Enter your username: ");
  fgets(username, sizeof(username), stdin);
  username[strcspn(username, "\n")] = '\0';  // 개행 문자 제거

  // 서버에 사용자 이름 전송
  send(client_socket, username, strlen(username), 0);

  // 채팅 시작
  printf("Type 'exit' to leave the chat.\n");

  while (1) {
    // 메시지 입력
    printf("[%s]: ", username);
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0';  // 개행 문자 제거

    // 종료 조건
    if (strcmp(message, "exit") == 0) {
      break;
    }

    // 서버에 메시지 전송
    send(client_socket, message, strlen(message), 0);
  }

  // 클라이언트 소켓 종료
  close(client_socket);

  return 0;
}
