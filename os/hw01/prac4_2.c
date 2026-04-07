#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sum = 0;

void *runner(void *param) {
    int start = atoi((char *)param);
    int end = start + 9999;

    for (int i = start; i <= end; i++) {
        sum += i;
    }
    pthread_exit(0);
}

int main() {
    pthread_t tid[4];
    char *starts[] = {"1", "10001", "20001", "30001"};

    for (int i = 0; i < 4; i++) {
        pthread_create(&tid[i], NULL, runner, starts[i]);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(tid[i], NULL);
    }

    printf("Final value of shared sum variable: %d\n", sum);

    return 0;
}