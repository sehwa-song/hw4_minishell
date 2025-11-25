#include <stdio.h>
#include <string.h>
#include <unistd.h>     // fork, execvp
#include <sys/wait.h>   // waitpid
#include <stdlib.h>     // exit

// 백그라운드 프로세스가 종료되었는지 확인하고 수거하는 함수
// 부모 프로세스가 주기적으로 호출하여 좀비 프로세스를 방지함
void check_background_processes() {
    int status;
    pid_t pid;

    // WNOHANG 을 사용하면, 종료된 자식만 즉시 회수하고
    // 아직 종료 안 된 자식은 기다리지 않고 바로 return
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[백그라운드 종료  pid=%d]\n", pid);
    }
}

int main() {

    char line[256];      // 한 줄 전체 입력 저장
    char *token;         // 명령어 첫 단어
    char *arg;           // 추가 인자 저장
    char *argv[64];      // execvp에 넘길 인자 배열
    int argc;            // 인자 개수

    printf("task3 shell\n");

    while (1) {

        // 백그라운드로 실행된 작업 중 이미 끝난 것이 있는지 확인하여 수거함
        check_background_processes();

        // 프롬프트 출력
        printf("task3$ ");

        // 사용자 입력 받기
        if (fgets(line, sizeof(line), stdin) == NULL)
            continue;

        // 개행문자 제거
        line[strcspn(line, "\n")] = '\0';

        // 공백 입력이면 다시 프롬프트
        if (strlen(line) == 0)
            continue;

        // 첫 토큰 추출 (명령어)
        token = strtok(line, " ");
        if (token == NULL)
            continue;

        //  여기서 quit 처리 
        // quit는 외부 명령이 아니라 "쉘 내부 명령"이므로
        // execvp로 넘기지 않고 즉시 프로그램 종료해야 함
        if (strcmp(token, "quit") == 0) {
            printf("task3 quit\n");
            exit(0);
        }

        // argv 배열 초기화 시작
        argc = 0;
        argv[argc++] = token;

        // 이후 토큰들을 반복해서 argv 배열에 저장
        while ((arg = strtok(NULL, " ")) != NULL) {
            argv[argc++] = arg;
        }

        // execvp는 마지막에 NULL로 끝나야 함
        argv[argc] = NULL;

        // 백그라운드 실행 여부 확인
        int background = 0;

        // 맨 마지막 인자가 "&"라면 백그라운드 명령
        if (argc > 0 && strcmp(argv[argc - 1], "&") == 0) {
            background = 1;
            argv[argc - 1] = NULL;   // execvp에서 & 제거
        }

        // 자식 프로세스 생성
        pid_t pid = fork();

        if (pid == 0) {
            // 자식 프로세스 
            // execvp로 외부 명령 실행
            execvp(argv[0], argv);

            // execvp가 실패하면 여기로 옴
            perror("exec 실행 실패");
            exit(1);
        }
        else if (pid > 0) {
            // 부모 프로세스 

            if (background) {
                // 백그라운드 실행은 기다리지 않고 바로 프롬프트로 돌아감
                printf("[백그라운드 실행  pid=%d]\n", pid);
            } else {
                // 포그라운드 실행은 자식이 끝날 때까지 기다림
                waitpid(pid, NULL, 0);
            }
        }
        else {
            // fork 자체 실패
            perror("fork 실패");
        }
    }

    return 0;
}

