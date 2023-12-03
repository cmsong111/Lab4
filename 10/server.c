// 서버는 채팅방을 생성하고 클라이언트들 간의 통신을 관리합니다.
// 사용자 등록, 메시지 전송, 파일 전송 등을 처리합니다.

// 채팅방 구성과 사용자 등록 기능을 가진 소켓 서버를 구현하기 위해서는 다음과
// 같은 단계를 따를 수 있습니다.

// 서버 소켓 생성 및 바인딩: 클라이언트의 연결을 받아들일 서버 소켓을 생성하고
// IP 주소와 포트를 바인딩합니다.

// 클라이언트 연결 대기: 클라이언트의 연결을 기다리는 대기 상태로 들어갑니다.

// 클라이언트 연결 수락: 클라이언트가 연결을 요청하면 수락하여 통신할 수 있는
// 소켓을 생성합니다.

// 채팅방 생성 및 사용자 등록: 채팅방을 생성하고 클라이언트가 채팅방에 참가할
// 때마다 사용자를 등록합니다.

// 메시지 수신 및 브로드캐스팅: 클라이언트로부터 메시지를 수신하면 해당 메시지를
// 다른 모든 클라이언트에게 전송하여 채팅을 구현합니다.

// 아래는 간단한 예제 코드입니다. 이 코드에서는 pthread 라이브러리를 사용하여 각
// 클라이언트에 대해 별도의 쓰레드를 생성하여 동시에 여러 클라이언트와 통신할 수
// 있게 합니다.

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// 사용자 정보 구조체
typedef struct {
  int socket;
  char username[50];
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];                    // 클라이언트 배열
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // 뮤텍스

// 클라이언트에게 메시지 전송
void send_message(char* message, int sender_socket) {
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i].socket > 0 && clients[i].socket != sender_socket) {
      send(clients[i].socket, message, strlen(message), 0);
    }
  }
  pthread_mutex_unlock(&mutex);
}

// 클라이언트 쓰레드 함수
void* handle_client(void* arg) {
  int client_socket = *(int*)arg;
  char buffer[BUFFER_SIZE];

  // 클라이언트 정보 입력
  printf("Enter username for client %d: ", client_socket);
  recv(client_socket, clients[client_socket].username,
       sizeof(clients[client_socket].username), 0);
  printf("Client %d connected with username: %s\n", client_socket,
         clients[client_socket].username);

  // 클라이언트에게 환영 메시지 전송
  char welcome_message[100];
  sprintf(welcome_message, "Welcome, %s!", clients[client_socket].username);
  send(client_socket, welcome_message, strlen(welcome_message), 0);

  // 채팅 메시지 수신 및 브로드캐스팅
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0) {
      // 클라이언트 연결 종료
      printf("Client %d disconnected\n", client_socket);
      close(client_socket);

      // 사용자 정보 초기화
      pthread_mutex_lock(&mutex);
      clients[client_socket].socket = 0;
      clients[client_socket].username[0] = '\0';
      pthread_mutex_unlock(&mutex);

      // 브로드캐스팅 메시지 전송
      char leave_message[100];
      sprintf(leave_message, "%s has left the chat.",
              clients[client_socket].username);
      send_message(leave_message, client_socket);

      pthread_exit(NULL);
    }

    // 클라이언트로부터 수신한 메시지 브로드캐스팅
    char broadcast_message[150];
    sprintf(broadcast_message, "%s: %s", clients[client_socket].username,
            buffer);
    send_message(broadcast_message, client_socket);
  }

  return NULL;
}

int main() {
  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  pthread_t client_threads[MAX_CLIENTS];

  // 클라이언트 배열 초기화
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    clients[i].socket = 0;
    clients[i].username[0] = '\0';
  }

  // 서버 소켓 생성
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 서버 주소 설정
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(8888);

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

  printf("Server listening on port 8888...\n");

  while (1) {
    // 클라이언트와 연결
    if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr,
                                &client_addr_len)) == -1) {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }

    // 빈 자리 찾기
    int i;
    for (i = 0; i < MAX_CLIENTS; ++i) {
      if (clients[i].socket == 0) {
        clients[i].socket = client_socket;
        break;
      }
    }

    if (i == MAX_CLIENTS) {
      // 클라이언트 수 초과
      printf("Too many clients. Connection rejected.\n");
      close(client_socket);
      continue;
    }

    // 클라이언트 쓰레드 생성
    if (pthread_create(&client_threads[i], NULL, handle_client,
                       (void*)&clients[i].socket) != 0) {
      perror("Thread creation failed");
      exit(EXIT_FAILURE);
    }

    // 쓰레드를 바로 종료하지 않도록 join을 호출하지 않음
    pthread_detach(client_threads[i]);
  }

  // 서버 소켓 종료
  close(server_socket);

  return 0;
}
