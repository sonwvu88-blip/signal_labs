
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define FILE_NAME "counter.txt"
#define ROUNDS 5

int read_last_counter(void)
{
    FILE *fp = fopen(FILE_NAME, "r");
    char line[256];
    int last_counter = 0;

    if (fp == NULL) {
        perror("fopen read");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        int pid_value;
        int counter_value;

        if (sscanf(line, "PID=%d counter=%d",
                   &pid_value, &counter_value) == 2) {
            last_counter = counter_value;
        }
    }

    fclose(fp);
    return last_counter;
}

void append_record(pid_t pid, int counter)
{
    FILE *fp = fopen(FILE_NAME, "a");

    if (fp == NULL) {
        perror("fopen append");
        exit(EXIT_FAILURE);
    }

    if (fprintf(fp, "PID=%d counter=%d\n",
                (int)pid, counter) < 0) {
        perror("fprintf");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    if (fclose(fp) == EOF) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }

    printf("PID=%d ghi counter=%d\n",
           (int)pid, counter);
    fflush(stdout);
}

void wait_for_signal(const sigset_t *set, int expected_signal)
{
    int received_signal;
    int error = sigwait(set, &received_signal);

    if (error != 0) {
        fprintf(stderr, "sigwait error: %d\n", error);
        exit(EXIT_FAILURE);
    }

    if (received_signal != expected_signal) {
        fprintf(stderr, "Received wrong signal\n");
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    sigset_t blocked_set;
    sigset_t wait_for_a;
    sigset_t wait_for_b;

    sigemptyset(&blocked_set);
    sigaddset(&blocked_set, SIGUSR1);
    sigaddset(&blocked_set, SIGUSR2);

    if (sigprocmask(SIG_BLOCK, &blocked_set, NULL) == -1) {
        perror("sigprocmask");
        return EXIT_FAILURE;
    }

    sigemptyset(&wait_for_a);
    sigaddset(&wait_for_a, SIGUSR1);

    sigemptyset(&wait_for_b);
    sigaddset(&wait_for_b, SIGUSR2);

    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        /* Process B */
        pid_t my_pid = getpid();

        for (int i = 0; i < ROUNDS; i++) {
            wait_for_signal(&wait_for_a, SIGUSR1);

            int counter = read_last_counter();
            counter++;

            append_record(my_pid, counter);

            if (kill(getppid(), SIGUSR2) == -1) {
                perror("kill SIGUSR2");
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }

    /* Process A */
    pid_t my_pid = getpid();

    for (int i = 0; i < ROUNDS; i++) {
        int counter = read_last_counter();
        counter++;

        append_record(my_pid, counter);

        if (kill(child_pid, SIGUSR1) == -1) {
            perror("kill SIGUSR1");
            return EXIT_FAILURE;
        }

        wait_for_signal(&wait_for_b, SIGUSR2);
    }

    int status;

    if (waitpid(child_pid, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Process B terminated abnormally\n");
        return EXIT_FAILURE;
    }

    printf("Completed synchronization between processes.\n");
    return EXIT_SUCCESS;
}
