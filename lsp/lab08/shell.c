#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

#define MAXARG 7

pid_t child_pid_global = -1;

void alarmHandler(int sig) {
    if (child_pid_global > 0) {
        printf("[알람] 자식 프로세스 %d 시간 초과\n", (int)child_pid_global);
        fflush(stdout);
        kill(child_pid_global, SIGINT);
    }
}

int main () {
    char buf[256];
    char *args[MAXARG];
    char *s, *save;
    char *cmd_ptr, *cmd_save;
    int argn;
    static const char delim[] = " \t\n";
    int pid, status;

    signal(SIGALRM, alarmHandler);

    while(1) {
        printf("[shell] ");
        fflush(stdout);

        if (fgets(buf, sizeof(buf), stdin) == NULL) break;

        cmd_ptr = strtok_r(buf, ";", &cmd_save);
        while(cmd_ptr != NULL) {
            argn = 0;

            char *pipe_pos = strchr(cmd_ptr, '|');
            if (pipe_pos != NULL) {
                *pipe_pos = '\0';
                char *cmd1_str = cmd_ptr;
                char *cmd2_str = pipe_pos + 1;

                char *args1[MAXARG], *args2[MAXARG];
                char *save1, *save2;
                int argn1 = 0, argn2 = 0;

                s = strtok_r(cmd1_str, delim, &save1);
                while (s != NULL && argn1 < MAXARG - 1) {
                    args1[argn1++] = s;
                    s = strtok_r(NULL, delim, &save1);
                }
                args1[argn1] = NULL;

                s = strtok_r(cmd2_str, delim, &save2);
                while (s != NULL && argn2 < MAXARG - 1) {
                    args2[argn2++] = s;
                    s = strtok_r(NULL, delim, &save2);
                }
                args2[argn2] = NULL;

                if (argn1 == 0 || argn2 == 0) {
                    fprintf(stderr, "pipe: 명령어가 없습니다\n");
                    cmd_ptr = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }

                int pipefd[2];
                if (pipe(pipefd) < 0) {
                    perror("pipe failed");
                    cmd_ptr = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }

                pid_t pid1 = fork();
                if (pid1 < 0) {
                    perror("fork failed");
                    close(pipefd[0]); close(pipefd[1]);
                    cmd_ptr = strtok_r(NULL, ";", &cmd_save);
                    continue;
                } else if (pid1 == 0) {
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);
                    execvp(args1[0], args1);
                    perror("execvp failed");
                    exit(1);
                }

                pid_t pid2 = fork();
                if (pid2 < 0) {
                    perror("fork failed");
                    close(pipefd[0]); close(pipefd[1]);
                    kill(pid1, SIGTERM);
                    waitpid(pid1, &status, 0);
                    cmd_ptr = strtok_r(NULL, ";", &cmd_save);
                    continue;
                } else if (pid2 == 0) {
                    close(pipefd[1]);
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);
                    execvp(args2[0], args2);
                    perror("execvp failed");
                    exit(1);
                }

                close(pipefd[0]);
                close(pipefd[1]);
                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);

                cmd_ptr = strtok_r(NULL, ";", &cmd_save);
                continue;
            }

            s = strtok_r(cmd_ptr, delim, &save);
            while(s != NULL && argn < MAXARG - 1) {
                args[argn++] = s;
                s = strtok_r(NULL, delim, &save);
            }
            args[argn] = NULL;

            if (argn == 0) {
                cmd_ptr = strtok_r(NULL, ";", &cmd_save);
                continue;
            }

            if (!strcmp(args[0], "quit")) exit(0);

            int limit = 0;
            if (isdigit((unsigned char)*args[0])) {
                limit = atoi(args[0]);
                for (int i = 0; i < argn; i++)
                    args[i] = args[i+1];
                argn--;
                if (argn == 0) {
                    fprintf(stderr, "timeout: 실행할 명령어가 없습니다\n");
                    cmd_ptr = strtok_r(NULL, ";", &cmd_save);
                    continue;
                }
            }

            int background = 0;
            if (argn > 0 && !strcmp(args[argn - 1] , "&")) {
                background = 1;
                args[--argn] = NULL;
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork failed");
            } else if (pid != 0) {
                if (limit > 0) {
                    child_pid_global = pid;
                    alarm(limit);
                }

                if (!background) {
                    while (waitpid(pid, &status, 0) == -1 && errno == EINTR);
                    printf("[%d] 자식 프로세스 %d 종료\n", (int)getpid(), (int)pid);
                    fflush(stdout);
                }

                if (limit > 0) {
                    alarm(0);
                    child_pid_global = -1;
                }
                
            } else {
                for (int i = 0; i < argn; i++) {
                    if (!strcmp(args[i], ">")) {
                        if (args[i + 1] == NULL) {
                            fprintf(stderr, "redirect: 파일명이 없습니다\n");
                            exit(1);
                        }
                        int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd < 0) {
                            perror("open failed");
                            exit(1);
                        }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                        args[i] = NULL;
                        args[i + 1] = NULL;
                        break;

                    } else if (!strcmp(args[i], "<")) {
                        if (args[i + 1] == NULL) {
                            fprintf(stderr, "redirect: 파일명이 없습니다\n");
                            exit(1);
                        }
                        int fd = open(args[i + 1], O_RDONLY);
                        if (fd < 0) {
                            perror("open failed");
                            exit(1);
                        }
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                        args[i] = NULL;
                        args[i + 1] = NULL;
                        break;
                    }
                }
                execvp(args[0], args);
                perror("execvp failed");
                exit(1);
            }
            cmd_ptr = strtok_r(NULL, ";", &cmd_save);
        }
    }
    return 0;
}