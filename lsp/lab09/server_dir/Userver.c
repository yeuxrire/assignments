#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define DEFAULT_PROTOCOL 0
#define MAXLINE 100

int readLine(int fd, char* str);

int main(int argc, char* argv[])
{
    int sfd, cfd, port, clientlen;
    FILE *fp;
    char inmsg[MAXLINE], outmsg[MAXLINE];
    struct sockaddr_in serveraddr, clientaddr;
    char *haddrp;

    signal(SIGCHLD, SIG_IGN);

    if (argc != 2) {
        fprintf(stderr, "사용법: %s <port>\n", argv[0]);
        exit(0);
    }

    port = atoi(argv[1]);
    sfd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    bind(sfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    listen(sfd, 5);

    while (1) {
        clientlen = sizeof(clientaddr);
        cfd = accept(sfd, (struct sockaddr *) &clientaddr, &clientlen);

        haddrp = inet_ntoa(clientaddr.sin_addr);
        printf("서버: %s (%d)에 연결됨\n", haddrp, clientaddr.sin_port);

        if (fork() == 0) {
            close(sfd); 

            readLine(cfd, inmsg);
            printf("업로드 파일명: %s\n", inmsg);

            fp = fopen(inmsg, "w");
            if (fp == NULL) {
                perror("fopen error");
                close(cfd);
                exit(1);
            }

            while (readLine(cfd, outmsg)) {
                fputs(outmsg, fp);
            }

            fclose(fp);
            close(cfd);
            printf("업로드 완료: %s\n", inmsg);
            exit(0);
        } else {
            close(cfd);
        }
    }
}

int readLine(int fd, char* str)
{
    int n;
    do {
        n = read(fd, str, 1);
    } while (n > 0 && *str++ != '\0');
    return(n > 0);
}