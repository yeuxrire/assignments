#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXARG 7

int main () {
    char buf[256];
    char *args[MAXARG];
    char *s, *save;
    char *cmd_ptr, *cmd_save;
    int argn;
    static const char delim[] = " \t\n";
    int pid, status;

    while(1) {
        printf("[shell] ");
        fflush(stdout);

        if (fgets(buf, sizeof(buf), stdin) == NULL) break;

        cmd_ptr = strtok_r(buf, ";", &cmd_save);
        while(cmd_ptr != NULL) {
            argn = 0;

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

            int background = 0;
            if (argn > 0 && !strcmp(args[argn - 1] , "&")) {
                background = 1;
                args[--argn] = NULL;
            }

            if ((pid = fork()) == -1) {
                perror("fork failed");
            } else if (pid != 0) {
                if (!background) {
                    waitpid(pid, &status, 0);
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