#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    char line[256];

    printf("task2 shell\n");

    while (1) {
        printf("task2$ ");

        // 한 줄 입력받기
        if (!fgets(line, sizeof(line), stdin))
            continue;

        // 줄 끝 개행 제거
        line[strcspn(line, "\n")] = '\0';

        // 빈 입력이면 다시 프롬프트
        if (strlen(line) == 0)
            continue;

        // quit 처리
        if (strcmp(line, "quit") == 0) {
            printf("종료합니다.\n");
            break;
        }

        //  문자열을 공백 기준으로 토큰 분리
        char *argv[64];
        int argc = 0;

        char *token = strtok(line, " ");
        while (token != NULL) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;

        // 입력/출력 리다이렉션 위치 찾기
        int in_pos = -1;   // < 위치
        int out_pos = -1;  // > 위치

        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "<") == 0) {
                in_pos = i;
            }
            if (strcmp(argv[i], ">") == 0) {
                out_pos = i;
            }
        }

        // 자식 프로세스 생성
        pid_t pid = fork();

        if (pid == 0) {
            // (자식 프로세스)
            // exec 실행 전에 리다이렉션을 처리해야 함

            // 입력 리다이렉션 처리
            if (in_pos != -1) {
                int fd = open(argv[in_pos + 1], O_RDONLY);
                if (fd < 0) {
                    perror("입력 파일 오류");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);

                // "<"와 파일이름을 execvp 인자에서 제거
                argv[in_pos] = NULL;
            }

            // 출력 리다이렉션 처리
            if (out_pos != -1) {
                int fd = open(argv[out_pos + 1],
                              O_WRONLY | O_CREAT | O_TRUNC,
                              0644);
                if (fd < 0) {
                    perror("출력 파일 오류");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);

                // ">"와 파일이름을 execvp 인자에서 제거
                if (out_pos < argc)
                    argv[out_pos] = NULL;
            }

            // execvp 실행
            execvp(argv[0], argv);

            // exec에 도달하면 오류
            perror("exec 실행 실패");
            exit(1);
        }

        else if (pid > 0) {
            // 부모 프로세스
            // 자식이 끝날 때까지 기다림
            waitpid(pid, NULL, 0);
        }

        else {
            perror("fork 실패");
        }
    }

    return 0;
}

