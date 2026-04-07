#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        perror("Fork failed.");
        exit(1);
    }
    else if (pid == 0) { // Child Process
        execlp("./sum", "./sum", NULL);
        perror("Fork failed.");
        exit(1);
    }
    else { // Parent Process
        wait(&status);
        if (WIFEXITED(status)) {
            int result = WEXITSTATUS(status);
            printf("Result of Sum(1 to 10): %d\n", result);
        }
    }
    return 0; 
}