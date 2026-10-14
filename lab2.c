
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main(void)
{
    sigset_t set;
    sigset_t current_mask;

    if (sigemptyset(&set) == -1) {
        perror("sigemptyset");
        return 1;
    }

    if (sigaddset(&set, SIGINT) == -1) {
        perror("sigaddset");
        return 1;
    }

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    if (sigprocmask(SIG_BLOCK, NULL, &current_mask) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("SIGINT blocked: %s\n",
           sigismember(&current_mask, SIGINT) == 1
               ? "YES" : "NO");

    printf("Press Ctrl+C to test.\n");
    printf("Press Ctrl+\\ to exit.\n");

    while (1) {
        pause();
    }

    return 0;
}
