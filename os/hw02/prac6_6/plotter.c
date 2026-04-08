#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h> 

#define N_COUNTER 4 
#define MILLI 1000

void mywrite(int n);
int myread();

pthread_mutex_t critical_section; 
sem_t semWrite, semRead; 
int queue[N_COUNTER]; 
int wIdx; 
int rIdx;

void* producer(void* arg) { 
    for(int i = 0; i < 10; i++) {
        int random_num = (rand() % 20) + 1; 
        mywrite(random_num);
        int m = rand() % 10; 
        usleep(MILLI * m * 10); 
    }
    return NULL;
}

void* consumer(void* arg) { 
    for(int i = 0; i < 10; i++) {
        int n = myread(); 
        for(int j = 0; j < n; j++) {
            printf("*");
        }
        printf("\n"); 
        int m = rand() % 10; 
        usleep(MILLI * m * 10); 
    }
    return NULL;
}

void mywrite(int n) { 
    sem_wait(&semWrite); 
    pthread_mutex_lock(&critical_section); 
    queue[wIdx] = n; 
    wIdx++;
    wIdx %= N_COUNTER;
    pthread_mutex_unlock(&critical_section); 
    sem_post(&semRead); 
}

int myread() { 
    sem_wait(&semRead); 
    pthread_mutex_lock(&critical_section); 
    int n = queue[rIdx]; 
    rIdx++;
    rIdx %= N_COUNTER;
    pthread_mutex_unlock(&critical_section); 
    sem_post(&semWrite); 
    return n;
}

int main() {
    pthread_t t[2];
    srand(time(NULL));
    pthread_mutex_init(&critical_section, NULL); 
    int res = sem_init(&semWrite, 0, N_COUNTER);
    if(res == -1) {
        printf("semaphore is not supported\n");
        return 0;
    }
    sem_init(&semRead, 0, 0); 
    pthread_create(&t[0], NULL, producer, NULL); 
    pthread_create(&t[1], NULL, consumer, NULL); 

    for(int i = 0; i < 2; i++)
        pthread_join(t[i], NULL); 

    sem_destroy(&semRead); 
    sem_destroy(&semWrite); 
    pthread_mutex_destroy(&critical_section); 

    return 0;
}