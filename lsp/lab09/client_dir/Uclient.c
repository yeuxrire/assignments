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

int main(int argc, char* argv[])
{
    int cfd, port, result;
    char *host, inmsg[MAXLINE], outmsg[MAXLINE];
    struct sockaddr_in serverAddr;
    struct hostent *hp;
    FILE *fp;

    if (argc != 3) {
        fprintf(stderr, "사용법: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = atoi(argv[2]);

    cfd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);

    if ((hp = gethostbyname(host)) == NULL)
        perror("gethostbyname error");

    printf("%s -> %s\n", host, inet_ntoa(*(struct in_addr*) hp->h_addr_list[0]));
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0],
          (char *)&serverAddr.sin_addr.s_addr, hp->h_length);
    serverAddr.sin_port = htons(port);

    do { 
        result = connect(cfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
        if (result == -1) sleep(1);
    } while (result == -1);


    printf("업로드할 파일 이름 입력: ");
    scanf("%s", inmsg);

    write(cfd, inmsg, strlen(inmsg) + 1);

    fp = fopen(inmsg, "r");
    if (fp == NULL) {
        perror("fopen error");
        close(cfd);
        exit(1);
    }

    while (fgets(outmsg, MAXLINE, fp) != NULL) {
        write(cfd, outmsg, strlen(outmsg) + 1);
    }

    fclose(fp);
    close(cfd);
    printf("업로드 완료: %s\n", inmsg);
    exit(0);
}