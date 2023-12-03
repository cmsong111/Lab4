#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define RESPONSE_TEMPLATE \
  "HTTP/1.1 200 OK\nContent-Length: %zu\nContent-Type: text/html\n\n%s"

void handle_get_request(int client_socket, const char* request_path) {
  char response_buffer[BUFFER_SIZE];
  char file_path[BUFFER_SIZE];
  FILE* file;

  // "/" 경로인 경우 기본 페이지로 지정
  if (strcmp(request_path, "/") == 0) {
    strcpy(file_path, "index.html");
  } else {
    // URL에서 '/' 제거하여 파일 경로로 사용
    sprintf(file_path, "%s", request_path + 1);
  }

  // 파일 열기 시도
  if ((file = fopen(file_path, "r")) == NULL) {
    // 파일이 없는 경우 404 에러 응답
    snprintf(response_buffer, BUFFER_SIZE,
             "HTTP/1.1 404 Not Found\nContent-Length: 13\n\nFile Not Found");
  } else {
    // 파일이 존재하는 경우 응답 생성
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* file_content = (char*)malloc(file_size + 1);
    fread(file_content, 1, file_size, file);
    fclose(file);
    file_content[file_size] = '\0';

    snprintf(response_buffer, BUFFER_SIZE, RESPONSE_TEMPLATE, file_size,
             file_content);

    free(file_content);
  }

  // 클라이언트에 응답 전송
  send(client_socket, response_buffer, strlen(response_buffer), 0);
}

void handle_post_request(int client_socket,
                         const char* request_path,
                         const char* post_data) {
  // 여기에 POST 메소드 처리 로직 추가
  // 현재는 단순히 수신한 데이터를 그대로 응답으로 전송하는 예제
  char response_buffer[BUFFER_SIZE];
  snprintf(response_buffer, BUFFER_SIZE, RESPONSE_TEMPLATE, strlen(post_data),
           post_data);
  send(client_socket, response_buffer, strlen(response_buffer), 0);
}

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

  printf("Web server listening on port %d...\n", PORT);

  while (1) {
    // 클라이언트와 연결
    if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr,
                                &client_addr_len)) == -1) {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }

    printf("Client connected\n");

    // 클라이언트로부터의 HTTP 요청 읽기
    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socket, buffer, sizeof(buffer), 0) == -1) {
      perror("Receive failed");
      exit(EXIT_FAILURE);
    }

    // HTTP 메소드 및 경로 추출
    char method[10];
    char path[255];
    sscanf(buffer, "%s %s", method, path);

    // POST 메소드인 경우 Content-Length를 이용하여 데이터 수신
    if (strcmp(method, "POST") == 0) {
      char* content_length_start = strstr(buffer, "Content-Length:");
      if (content_length_start != NULL) {
        int content_length;
        sscanf(content_length_start, "Content-Length: %d", &content_length);

        // 데이터를 읽기 위한 버퍼
        char post_data[BUFFER_SIZE];
        int total_read = 0;

        // Content-Length만큼 데이터 수신
        while (total_read < content_length) {
          int bytes_read = recv(client_socket, post_data + total_read,
                                content_length - total_read, 0);
          if (bytes_read == -1) {
            perror("Receive failed");
            exit(EXIT_FAILURE);
          }
          total_read += bytes_read;
        }

        post_data[total_read] = '\0';

        // POST 메소드 처리 함수 호출
        handle_post_request(client_socket, path, post_data);
      }
    } else if (strcmp(method, "GET") == 0) {
      // GET 메소드 처리 함수 호출
      handle_get_request(client_socket, path);
    }

    // 클라이언트 소켓 종료
    close(client_socket);
    printf("Client disconnected\n");
  }

  // 서버 소켓 종료
  close(server_socket);

  return 0;
}
