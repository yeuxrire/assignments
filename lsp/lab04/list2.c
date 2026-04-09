#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

char type(mode_t);
char *perm(mode_t);
void printStat(char*, char*, struct stat*, int, int, int, int);

/* 디렉터리 내용을 자세히 리스트한다. */
int main(int argc, char **argv)
{
    DIR *dp;
    char *dir = ".";
    struct stat st;
    struct dirent *d;
    char path[BUFSIZ+1];

    int opt_a = 0, opt_l = 0, opt_i = 0, opt_s = 0, opt_r = 0;
    
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]){
                    case 'a': opt_a = 1; break;
                    case 'l': opt_l = 1; break;
                    case 'i': opt_i = 1; break;
                    case 's': opt_s = 1; break;
                    case 'r': opt_r = 1; break;
                    default:
                        fprintf(stderr, "Usage: %s[-alisr] [directory]\n", argv[0]);
                        exit(1);
                }
            }
        } else {
            dir = argv[i];
        }
    }

    if ((dp = opendir(dir)) == NULL) {// 디렉터리 열기
        perror(dir);
        exit(1);
    }

    char *file_list[1024];
    int file_count = 0;
    
    while ((d = readdir(dp)) != NULL) { 
        if (opt_a == 0 && d->d_name[0] == '.') {
            continue;
        }
        if (file_count < 1024) {
            file_list[file_count] =strdup(d->d_name);
            file_count++;
        } else {
            fprintf(stderr, "경고: 파일이 1024개를 초과하여 일부만 읽습니다.\n");
            break;
        }
    }
    closedir(dp);

    if (opt_r) {
        for (int i = file_count -1; i >= 0; i--) {
            sprintf(path, "%s/%s", dir, file_list[i]);
            if (lstat(path, &st) < 0)
                perror(path);
            else
                printStat(path, file_list[i], &st, opt_a, opt_l, opt_i, opt_s);
            
            free(file_list[i]);
        }
    } else {
        for (int i = 0; i < file_count; i++) {
            sprintf(path, "%s/%s", dir, file_list[i]);
            if (lstat(path, &st) < 0)
                perror(path);
            else
                printStat(path, file_list[i], &st, opt_a, opt_l, opt_i, opt_s);
            
            free(file_list[i]);
        }
    }
    exit(0);
}

/* 파일 상태 정보를 출력 */
void printStat(char *pathname, char *file, struct stat *st, int opt_a, int opt_l, int opt_i, int opt_s)
{
    if (opt_i) {
        printf("%llu ", (unsigned long long)st->st_ino);
    }

    if (opt_s) {
        printf("%4ld ", (long)(st->st_blocks / 2));
    }

    if (opt_l) {
    printf("%c%s ", type(st->st_mode), perm(st->st_mode));
    printf("%3lu ", (unsigned long)st->st_nlink);
    printf("%s %s ", getpwuid(st->st_uid)->pw_name, getgrgid(st->st_gid)->gr_name);
    printf("%9lld ", (long long)st-> st_size);
    printf("%.12s ", ctime(&st->st_mtime)+4);
    }

    printf("%s\n", file);
    
}

/* 파일 타입을 반환 */
char type(mode_t mode)
{
    if (S_ISREG(mode))
        return('-');
    if (S_ISDIR(mode))
        return('d');
    if (S_ISCHR(mode))
        return('c');
    if (S_ISBLK(mode))
        return('b');
    if (S_ISLNK(mode))
        return('l');
    if (S_ISFIFO(mode))
        return('p');
    if (S_ISSOCK(mode))
        return('s');
}

/* 파일 접근권한을 반환 */
char* perm(mode_t mode)
{
    static char perms[10];
    strcpy(perms, "---------");

    for (int i = 0; i < 3; i++) {
        if (mode & (S_IRUSR >> i*3))
            perms[i*3] = 'r';
        if (mode & (S_IWUSR >> i*3))
            perms[i*3+1] = 'w';
        if (mode & (S_IXUSR >> i*3))
            perms[i*3+2] = 'x';
    }
    return(perms);
}