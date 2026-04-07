#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sum [4] = {0, 0, 0, 0};

void *runner(void *param) {
    int id = atoi((char *)param);
    int start = (id - 1) * 10000 + 1;
    int end = id * 10000;

    for (int i = start; i <= end; i++) {
        sum[id-1] += i;
    }
    pthread_exit(0);
}
int main() {
    pthread_t tid[4];
    char *ids[] = {"1", "2", "3", "4"};

    for (int i = 0; i < 4; i++) {
        pthread_create(&tid[i], NULL, runner, ids[i]);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(tid[i], NULL);
    }

    int total_sum = sum[0] + sum[1] + sum[2] + sum[3];

    printf("Total Sum (1 to 40000) from 4 threads: %d\n", total_sum);

    return 0;
}