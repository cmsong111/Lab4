#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// 쓰레드가 수행할 함수
void* thread_function(void* arg) {
  int* thread_arg = (int*)arg;
  printf("This is thread function with argument: %d\n", *thread_arg);
  return NULL;
}

int main() {
  pthread_t thread_id;
  int argument_value = 42;

  // 쓰레드 생성
  if (pthread_create(&thread_id, NULL, thread_function,
                     (void*)&argument_value) != 0) {
    fprintf(stderr, "Error creating thread\n");
    exit(EXIT_FAILURE);
  }

  // 메인 쓰레드에서도 일부 동작 수행
  printf("This is the main thread.\n");

  // 쓰레드의 종료를 기다림
  if (pthread_join(thread_id, NULL) != 0) {
    fprintf(stderr, "Error joining thread\n");
    exit(EXIT_FAILURE);
  }

  printf("Main thread and created thread have both finished.\n");

  return 0;
}
