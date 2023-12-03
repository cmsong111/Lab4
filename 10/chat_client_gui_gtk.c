// 채팅 클라이언트를 GUI 기반으로 구현합니다. 여기에서는 GTK+를 사용하는 예제를
// 제시합니다.
#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

GtkWidget* text_view;
GtkWidget* entry;
int client_socket;

// 메시지 수신 쓰레드 함수
void* receive_thread(void* data) {
  char buffer[BUFFER_SIZE];
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socket, buffer, sizeof(buffer), 0) > 0) {
      // 서버로부터 메시지를 받아 텍스트 뷰에 추가
      GtkTextBuffer* buffer =
          gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
      GtkTextIter iter;
      gtk_text_buffer_get_end_iter(buffer, &iter);
      gtk_text_buffer_insert(buffer, &iter, buffer, -1);
    }
  }
}

// 전송 버튼 클릭 이벤트 핸들러
void on_send_button_clicked(GtkButton* button, gpointer user_data) {
  const gchar* message = gtk_entry_get_text(GTK_ENTRY(entry));
  if (strlen(message) > 0) {
    send(client_socket, message, strlen(message), 0);
    gtk_entry_set_text(GTK_ENTRY(entry), "");  // 메시지 전송 후 엔트리 초기화
  }
}

int main(int argc, char* argv[]) {
  GtkWidget* window;
  GtkWidget* vbox;
  GtkWidget* scrolled_window;
  GtkWidget* send_button;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <Server IP>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // 클라이언트 소켓 생성
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 서버 주소 설정
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);  // 서버 IP 주소
  server_addr.sin_port = htons(8888);                // 서버 포트 번호

  // 서버에 연결
  if (connect(client_socket, (struct sockaddr*)&server_addr,
              sizeof(server_addr)) == -1) {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  // GTK+ 초기화
  gtk_init(&argc, &argv);

  // 창 생성
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Chat Client");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // 수직 박스 생성
  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  // 텍스트 뷰 생성
  text_view = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
  gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

  // 엔트리 생성
  entry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 0);

  // 전송 버튼 생성
  send_button = gtk_button_new_with_label("Send");
  g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked),
                   NULL);
  gtk_box_pack_start(GTK_BOX(vbox), send_button, FALSE, TRUE, 0);

  // 수신 쓰레드 생성 및 시작
  pthread_t receive_thread_id;
  pthread_create(&receive_thread_id, NULL, receive_thread, NULL);

  // GTK+ 메인 루프 시작
  gtk_widget_show_all(window);
  gtk_main();

  // 클라이언트 소켓 종료
  close(client_socket);

  return 0;
}
