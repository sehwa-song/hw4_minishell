#include <stdio.h>
#include <string.h>
#include <unistd.h>     // fork, execvp
#include <sys/wait.h>   // waitpid

// 허용된 명령어 목록인지 확인하는 함수
int is_allowed_command(const char *cmd) {
    if (strcmp(cmd, "echo") == 0) return 1;
    if (strcmp(cmd, "ls") == 0) return 1;
    if (strcmp(cmd, "sort") == 0) return 1;
    if (strcmp(cmd, "sleep") == 0) return 1;
    if (strcmp(cmd, "./train") == 0) return 1;
    if (strcmp(cmd, "quit") == 0) return 1;
    return 0;
}

int main() {
    char line[256];
    char *token;

    printf("minishell\n");

    while (1) {
        printf("minishell$ ");

        // 입력 한 줄 받기
        fgets(line, sizeof(line), stdin);

        // 마지막에 붙는 개행문자 제거
        line[strcspn(line, "\n")] = '\0';

        // 비어있는 입력은 다시 프롬프트로
        if (strlen(line) == 0)
            continue;

        // quit 처리
        if (strcmp(line, "quit") == 0) {
            printf("minishell quit\n");
            break;
        }

        // 입력받은 문자열을 공백 기준으로 분리해서 첫 번째 단어는 명령어로 사용
        token = strtok(line, " ");
        if (token == NULL)
            continue;

        // execvp로 넘길 배열 준비
        char *argv[64];
        int argc = 0;

        argv[argc++] = token;

        // 나머지 단어들은 인자로 저장
        char *arg;
        while ((arg = strtok(NULL, " ")) != NULL) {
            argv[argc++] = arg;
        }
        argv[argc] = NULL;

        // 허용되지 않은 명령 입력 시 에러 처리
        if (!is_allowed_command(argv[0])) {
            printf("[ERROR] 허용되지 않은 명령어입니다.\n");
            continue;
        }

        // 자식 프로세스를 만들어 실제 리눅스 명령 실행
        pid_t pid = fork();

        if (pid == 0) {
            // 자식은 exec로 명령 실행
            execvp(argv[0], argv);

            // exec 실패 시 여기 도달
            perror("execvp 오류");
            return 1;
        }
        else if (pid > 0) {
            // 부모는 자식 종료까지 대기
            waitpid(pid, NULL, 0);
        }
        else {
            perror("fork 오류");
        }
    }

    return 0;
}

