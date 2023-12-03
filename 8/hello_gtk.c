#include <gtk/gtk.h>

// 버튼 클릭 이벤트 핸들러
static void button_clicked(GtkWidget* widget, gpointer data) {
  g_print("Hello, GTK!\n");
}

int main(int argc, char* argv[]) {
  // GTK 초기화
  gtk_init(&argc, &argv);

  // 윈도우 생성
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Hello, GTK!");

  // 버튼 생성
  GtkWidget* button = gtk_button_new_with_label("Click me!");

  // 버튼 클릭 이벤트 시그널 연결
  g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), NULL);

  // 레이아웃 설정
  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add(GTK_CONTAINER(box), button);
  gtk_container_add(GTK_CONTAINER(window), box);

  // 윈도우 크기 및 종료 설정
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // 윈도우 및 위젯 표시
  gtk_widget_show_all(window);

  // 메인 이벤트 루프 실행
  gtk_main();

  return 0;
}
