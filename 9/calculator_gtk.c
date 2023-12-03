#include <gtk/gtk.h>

// 계산기 상태를 나타내는 열거형
typedef enum {
  OPERATOR_NONE,
  OPERATOR_ADD,
  OPERATOR_SUBTRACT,
  OPERATOR_MULTIPLY,
  OPERATOR_DIVIDE
} Operator;

// 전역 변수
double current_value = 0.0;
Operator current_operator = OPERATOR_NONE;

// 버튼 클릭 이벤트 핸들러
static void button_clicked(GtkWidget* widget, gpointer data) {
  // 버튼의 레이블 가져오기
  const gchar* label = gtk_button_get_label(GTK_BUTTON(widget));

  // 숫자 버튼인 경우
  if (g_ascii_isdigit(label[0])) {
    double digit = atof(label);
    current_value = current_value * 10 + digit;
  } else {
    // 연산자 버튼인 경우
    switch (label[0]) {
      case '+':
        current_operator = OPERATOR_ADD;
        break;
      case '-':
        current_operator = OPERATOR_SUBTRACT;
        break;
      case '*':
        current_operator = OPERATOR_MULTIPLY;
        break;
      case '/':
        current_operator = OPERATOR_DIVIDE;
        break;
      case '=':
        // 현재까지 입력된 값과 연산자에 따라 계산
        switch (current_operator) {
          case OPERATOR_ADD:
            current_value += current_value;
            break;
          case OPERATOR_SUBTRACT:
            current_value -= current_value;
            break;
          case OPERATOR_MULTIPLY:
            current_value *= current_value;
            break;
          case OPERATOR_DIVIDE:
            if (current_value != 0.0) {
              current_value /= current_value;
            } else {
              // 0으로 나누는 경우 에러 처리
              g_print("Error: Division by zero\n");
              current_value = 0.0;
            }
            break;
          case OPERATOR_NONE:
            // 연산자가 없는 경우 현재값을 유지
            break;
        }
        break;
      case 'C':
        // Clear 버튼인 경우 초기화
        current_value = 0.0;
        current_operator = OPERATOR_NONE;
        break;
    }
  }

  // 결과를 표시하기 위해 버튼 레이블로 설정
  gchar result_text[20];
  snprintf(result_text, sizeof(result_text), "%.2f", current_value);
  gtk_entry_set_text(GTK_ENTRY(data), result_text);
}

int main(int argc, char* argv[]) {
  // GTK 초기화
  gtk_init(&argc, &argv);

  // 윈도우 생성
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Simple Calculator");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 300);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // 그리드 생성
  GtkWidget* grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);

  // 결과를 표시할 엔트리 위젯 생성
  GtkWidget* entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entry), "0.00");
  gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 4, 1);

  // 버튼 생성 및 이벤트 핸들러 연결
  gchar* button_labels[] = {"7", "8", "9", "/", "4", "5", "6", "*",
                            "1", "2", "3", "-", "0", ".", "=", "+"};

  for (int i = 0; i < 16; i++) {
    GtkWidget* button = gtk_button_new_with_label(button_labels[i]);
    g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), entry);
    gtk_grid_attach(GTK_GRID(grid), button, i % 4, 1 + i / 4, 1, 1);
  }

  // 윈도우 및 위젯 표시
  gtk_widget_show_all(window);

  // 메인 이벤트 루프 실행
  gtk_main();

  return 0;
}
