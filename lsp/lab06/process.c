#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

extern char **__environ;

int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "사용법: %s [-e [이름]] | [-u] | [-g] | [-i] | [-p]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-e") == 0) {
        if (argc == 2) {
            for (char **env = __environ; *env != 0; env++) {
                printf("%s\n", *env);
            }
        } else if (argc == 3) {
            char *env_val = getenv(argv[2]);
            if (env_val != NULL) {
                printf("%s=%s\n", argv[2], env_val);
            } else {
                printf("%s = (null)\n", argv[2]);
            }
        }
    }

    else if (strcmp(argv[1], "-u") == 0) {
        uid_t ruid = getuid();
        uid_t euid = geteuid();
        struct passwd *pw_ruid = getpwuid(ruid);
        struct passwd *pw_euid = getpwuid(euid);

        printf("나의 실제 사용자 ID: %d(%s)\n", ruid, pw_ruid ? pw_ruid->pw_name : "");
        printf("나의 유효 사용자 ID: %d(%s)\n", euid, pw_euid ? pw_euid->pw_name : "");
    }

    else if (strcmp(argv[1], "-g") == 0) {
        gid_t rgid = getgid();
        gid_t egid = getegid();
        struct group *gr_rgid = getgrgid(rgid);
        struct group *gr_egid = getgrgid(egid);

        printf("나의 실제 그룹 ID: %d(%s)\n", rgid, gr_rgid ? gr_rgid->gr_name : "");
        printf("나의 유효 그룹 ID: %d(%s)\n", egid, gr_egid ? gr_egid->gr_name : "");
    }

    else if (strcmp(argv[1], "-i") == 0) {
        printf("나의 프로세스 번호: [%d]\n", getpid());
    }

    else if (strcmp(argv[1], "-p") == 0) {
        printf("내 부모 프로세스 번호: [%d]\n", getppid());
    }

    else {
        printf("알 수 없는 옵션입니다.\n");
    }

    return 0;
}