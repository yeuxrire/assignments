#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define START_ACC 1000

struct account {
    int acc_no;
    char name[50];
    int balance;
    int password;
    time_t created_at;
    
};

void set_record_lock(int fd, int lock_type, int acc_no){
    struct flock lock;

    lock.l_type = lock_type;
    lock.l_whence = SEEK_SET;
    lock.l_start = (acc_no - START_ACC) * sizeof(struct account);
    lock.l_len = sizeof(struct account);

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("레코드 잠금/해제 오류");
        exit(1);
    }
}

int main() {
    int fd;
    int choice;
    int acc_no, target_acc, input_pw;
    int amount;
    struct account acc, target;
    char filename[] = "bank_account_final.dat";

    if ((fd = open(filename, O_RDWR | O_CREAT, 0644)) == -1) {
        perror("파일 열기 오류");
        exit(2);
    }

    while(1) {
        printf("1. 계좌 생성\n2. 입금\n3. 출금\n4. 잔액 조회\n5. 계좌 이체\n6. 계좌 해지\n7.종료\n");
        printf("선택: ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 7) {
            printf("프로그램을 종료합니다.\n");
            break;
        }

        printf("계좌 번호(1000~1999): ");
        scanf("%d", &acc_no);

        switch (choice) {
            case 1:
                set_record_lock(fd, F_WRLCK, acc_no);
                lseek(fd, (acc_no - START_ACC) * sizeof(acc), SEEK_SET);
                if (read(fd, &acc, sizeof(acc)) > 0 && acc.acc_no != 0) {
                    printf("오류: 이미 존재하는 계좌 번호입니다.\n");
                } else {
                    acc.acc_no = acc_no;
                    printf("소유자 이름: ");
                    scanf("%s", acc.name);
                    printf("비밀번호 (숫자 4자리): ");
                    scanf("%d", &acc.password);
                    printf("초기 입금액: ");
                    scanf("%d", &acc.balance);
                    time(&acc.created_at);
                    lseek(fd, (acc_no - START_ACC) * sizeof(acc), SEEK_SET);
                    write(fd, &acc, sizeof(acc));
                    printf("계좌가 성공적으로 생성되었습니다.\n");
                }
                set_record_lock(fd, F_UNLCK, acc_no);
                break;

            case 2:
                set_record_lock(fd, F_WRLCK, acc_no);
                lseek(fd, (acc_no - START_ACC) * sizeof(acc), SEEK_SET);
                if (read(fd, &acc, sizeof(acc)) > 0 && acc.acc_no != 0) {
                    printf("입금액: ");
                    scanf("%d", &amount);
                    acc.balance += amount;
                    lseek(fd, -sizeof(acc), SEEK_CUR);
                    write(fd, &acc, sizeof(acc));
                    printf("입금이 완료되었습니다. 현재 잔액: %d원\n", acc.balance);
                } else {
                    printf("존재하지 않는 계좌입니다.\n");
                }
                set_record_lock(fd, F_UNLCK, acc_no);
                break;

            case 3:
                set_record_lock(fd, F_WRLCK, acc_no);
                lseek(fd, (acc_no - START_ACC) * sizeof(acc), SEEK_SET);
                if (read(fd, &acc, sizeof(acc)) > 0 && acc.acc_no != 0) {
                    printf("비밀번호: ");
                    scanf("%d", &input_pw);
                    if (acc.password == input_pw) {
                        printf("출금액: ");
                        scanf("%d", &amount);
                        if (acc.balance >= amount) {
                            acc.balance -= amount;
                            lseek(fd, -sizeof(acc), SEEK_CUR);
                            write(fd, &acc, sizeof(acc));
                            printf("출금이 완료되었습니다. 현재 잔액:%d원\n", acc.balance);
                        } else {
                            printf("오류: 잔액이 부족합니다.\n");
                        }
                    } else {
                        printf("오류: 비밀번호가 틀렸습니다.\n");
                    }
                } else {
                    printf("존재하지 않는 계좌입니다.\n");
                }
                set_record_lock(fd, F_UNLCK, acc_no);
                break;

            case 4:
                set_record_lock(fd, F_RDLCK, acc_no);
                lseek(fd, (acc_no - START_ACC) * sizeof(acc), SEEK_SET);
                if (read(fd, &acc, sizeof(acc)) > 0 && acc.acc_no != 0) {
                    struct tm *t_info = localtime(&acc.created_at);
                    char time_str[50];
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t_info);
                    printf("계좌번호: %d\n", acc.acc_no);
                    printf("소유자명: %s\n", acc.name);
                    printf("개설일자: %s\n", time_str);
                    printf("현재잔액: %d\n", acc.balance);
                } else {
                    printf("존재하지 않는 계좌입니다.\n");
                }
                set_record_lock(fd, F_UNLCK, acc_no);
                break;

            case 5:
                printf("비밀번호: ");
                scanf("%d", &input_pw);
                printf("입금할 계좌번호 (1000~1999): ");
                scanf("%d", &target_acc);
                printf("이체 금액: ");
                scanf("%d", &amount);

                int first_lock = (acc_no < target_acc) ? acc_no : target_acc;
                int second_lock = (acc_no < target_acc) ? target_acc : acc_no;

                set_record_lock(fd, F_WRLCK, first_lock);
                set_record_lock(fd, F_WRLCK, second_lock);

                lseek(fd, (acc_no - START_ACC) * sizeof(acc), SEEK_SET);
                int sender_exists = read(fd, &acc, sizeof(acc));

                lseek(fd, (acc_no - START_ACC) * sizeof(target), SEEK_SET);
                int receiver_exists = read(fd, &target, sizeof(target));

                if (sender_exists > 0 && acc.acc_no != 0 && receiver_exists > 0 && target.acc_no != 0) {
                    if (acc.password == input_pw) {
                        if (acc.balance >= amount) {
                            acc.balance -= amount;
                            target.balance += amount;

                            lseek(fd, (acc_no - START_ACC) * sizeof(acc), SEEK_SET);
                            write(fd, &acc, sizeof(acc));

                            lseek(fd, (target_acc - START_ACC) * sizeof(target), SEEK_SET);
                            write(fd, &target, sizeof(target));

                            printf("이체가 성공적으로 완료되었습니다. 현재 잔액: %d원\n", acc.balance);
                        } else {
                            printf("오류: 잔액이 부족합니다.\n");
                        }
                    } else {
                        printf("오류: 비밀번호가 틀렸습니다.\n");
                    }
                } else {
                    printf("송금인 또는 수취인의 계좌가 존재하지 않습니다.\n");
                }

                set_record_lock(fd, F_UNLCK, first_lock);
                set_record_lock(fd, F_UNLCK, second_lock);
                break;

            case 6:
                set_record_lock(fd, F_WRLCK, acc_no);
                lseek(fd, (acc_no - START_ACC) * sizeof(acc), SEEK_SET);

                if (read(fd, &acc, sizeof(acc)) > 0 && acc.acc_no != 0) {
                    printf("비밀번호: ");
                    scanf("%d", &input_pw);
                    if (acc.password == input_pw) {
                        if (acc.balance > 0) {
                            printf("잔액 %d원을 모두 출금한 후 해지 처리합니다.\n", acc.balance);
                        }
                        memset(&acc, 0, sizeof(acc));
                        lseek(fd, (acc_no - START_ACC) * sizeof(acc), SEEK_SET);
                        write(fd, &acc, sizeof(acc));
                        printf("계좌가 해지되었습니다.\n");
                    } else {
                        printf("오류: 비밀번호가 틀렸습니다.\n");
                    }
                } else {
                    printf("존재하지 않는 계좌입니다.\n");
                }
                set_record_lock(fd, F_UNLCK, acc_no);
                break;

            default:
                printf("잘못된 선택입니다. 메뉴 번호를 다시 입력하세요.\n");
        }
    }

    close(fd);
    return 0;

}