#include <stdio.h>
#include <pthread.h>

int sum = 0; // 두 스레드가 공유하는  변수

void* worker(void* arg) { // 스레드 코드
	for(int i=0; i<1000000; i++) {
		sum = sum + 10;
	}
}

int main() {
	char *name[] = {"황기태", "이찬수"};
	pthread_t tid[2]; // 2개의 스레드 ID를 담을 배열
	pthread_attr_t attr[2]; // 2개의 스레드 정보를 담을 배열 

	pthread_attr_init(&attr[0]); // 디폴트 속성으로 초기화
	pthread_attr_init(&attr[1]); // 디폴트 속성으로 초기화

	pthread_create(&tid[0], &attr[0], worker, name[0]); // 스레드 생성
	pthread_create(&tid[1], &attr[1], worker, name[1]); // 스레드 생성

	pthread_join(tid[0], NULL); // 스레드 종료 대기
	pthread_join(tid[1], NULL); // 스레드 종료 대기

	printf("total sum = %d\n", sum); // 두 스레드의 종료 후 sum 출력

	return 0;
}
