// 채팅 클라이언트에 파일 전송 기능을 추가합니다. 파일 전송은 서버를 통해
// 이루어질 수 있습니다.

// 파일 전송은 복잡한 기능이므로 좀 더 신경을 써야 합니다. 아래는 간단한 파일
// 전송 기능을 가진 예제 코드입니다. 이 코드는 채팅 클라이언트에서 파일을
// 선택하여 서버로 전송하고, 서버는 받은 파일을 저장합니다.

#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

GtkWidget* file_chooser;
GtkWidget* text_view;
int client_socket;

// 파일 전송 함수
void send_file(const char* file_path) {
  FILE* file = fopen(file_path, "rb");
  if (!file) {
    perror("File open failed");
    return;
  }

  char buffer[BUFFER_SIZE];
  while (1) {
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);
    if (bytesRead > 0) {
      send(client_socket, buffer, bytesRead, 0);
    }

    if (feof(file)) {
      break;
    }
  }

  fclose(file);
}

// 파일 선택 다이얼로그를 띄우고 선택한 파일의 경로를 반환
char* get_selected_file_path() {
  char* file_path = NULL;
  GtkWidget* dialog = gtk_file_chooser_dialog_new(
      "Select File", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel",
      GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
  }

  gtk_widget_destroy(dialog);

  return file_path;
}

// 전송 버튼 클릭 이벤트 핸들러
void on_send_button_clicked(GtkButton* button, gpointer user_data) {
  char* file_path = get_selected_file_path();
  if (file_path != NULL) {
    // 파일 전송 함수 호출
    send_file(file_path);
    g_free(file_path);
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
  gtk_window_set_title(GTK_WINDOW(window), "File Transfer Client");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // 수직 박스 생성
  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  // 파일 전송 버튼 생성
  send_button = gtk_button_new_with_label("Send File");
  g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked),
                   NULL);
  gtk_box_pack_start(GTK_BOX(vbox), send_button, FALSE, TRUE, 0);

  // GTK+ 메인 루프 시작
  gtk_widget_show_all(window);
  gtk_main();

  // 클라이언트 소켓 종료
  close(client_socket);

  return 0;
}
