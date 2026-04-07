#include <stdio.h>
#include <pthread.h>

int sum = 0; // 두 스레드가 공유하는  변수
pthread_spinlock_t lock; // 스핀락 변수 선언

void* worker(void* arg) { // 스레드 코드
	printf("%s 시작\t %d\n", (char*)arg, sum);
	for(int i=0; i<1000000; i++) {
		pthread_spin_lock(&lock); // entry 코드. 스핀락 잠그기
		sum = sum + 10; // 임계구역코드
		pthread_spin_unlock(&lock); // exit 코드. 스핀락 열기 
	}
	printf("%s 끝\t %d\n", (char*)arg, sum);
}
int main() {
	char *name[] = {"황기태", "이찬수"};
	pthread_t tid[2];
	pthread_attr_t attr[2]; // 스레드 정보를 담을 구조체

	pthread_attr_init(&attr[0]); // 디폴트 속성으로 초기화
	pthread_attr_init(&attr[1]); // 디폴트 속성으로 초기화

	pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE); // 스핀락 변수lock 초기화
	// lock 변수에 대한 접근을 현재 프로세스에 속한 스레드들로 제한

	pthread_create(&tid[0], &attr[0], worker, name[0]);  // 스레드 생성
	pthread_create(&tid[1], &attr[1], worker, name[1]); // 스레드 생성

	pthread_join(tid[0], NULL); // 스레드 종료 대기
	pthread_join(tid[1], NULL); // 스레드 종료 대기

	printf("최종 sum = %d\n", sum); // 두 스레드 종료 후 sum 출력

	pthread_spin_destroy(&lock); // 스핀락 lock 사용 끝

	return 0;
}

