#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
sem_t empty, full;
pthread_mutex_t mutex;

// 생산자 쓰레드 함수
void* producer(void* arg) {
  int item;
  for (int i = 0; i < 10; ++i) {
    item = rand() % 100;  // 생산되는 아이템은 임의의 숫자로 가정
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    // 생산자가 버퍼에 아이템을 추가
    printf("Produced: %d\n", item);
    buffer[i % BUFFER_SIZE] = item;

    pthread_mutex_unlock(&mutex);
    sem_post(&full);
  }
  pthread_exit(NULL);
}

// 소비자 쓰레드 함수
void* consumer(void* arg) {
  int item;
  for (int i = 0; i < 10; ++i) {
    sem_wait(&full);
    pthread_mutex_lock(&mutex);

    // 소비자가 버퍼에서 아이템을 제거
    item = buffer[i % BUFFER_SIZE];
    printf("Consumed: %d\n", item);

    pthread_mutex_unlock(&mutex);
    sem_post(&empty);
  }
  pthread_exit(NULL);
}

int main() {
  pthread_t producer_thread1, producer_thread2, consumer_thread1,
      consumer_thread2;

  // 세마포어 및 뮤텍스 초기화
  sem_init(&empty, 0, BUFFER_SIZE);
  sem_init(&full, 0, 0);
  pthread_mutex_init(&mutex, NULL);

  // 생산자와 소비자 쓰레드 생성
  pthread_create(&producer_thread1, NULL, producer, NULL);
  pthread_create(&producer_thread2, NULL, producer, NULL);
  pthread_create(&consumer_thread1, NULL, consumer, NULL);
  pthread_create(&consumer_thread2, NULL, consumer, NULL);

  // 쓰레드의 종료를 기다림
  pthread_join(producer_thread1, NULL);
  pthread_join(producer_thread2, NULL);
  pthread_join(consumer_thread1, NULL);
  pthread_join(consumer_thread2, NULL);

  // 세마포어 및 뮤텍스 해제
  sem_destroy(&empty);
  sem_destroy(&full);
  pthread_mutex_destroy(&mutex);

  return 0;
}
