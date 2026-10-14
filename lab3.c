
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main(void)
{
    sigset_t block_set;
    sigset_t pending_set;

    if (sigemptyset(&block_set) == -1) {
        perror("sigemptyset");
        return 1;
    }

    if (sigaddset(&block_set, SIGINT) == -1) {
        perror("sigaddset");
        return 1;
    }

    if (sigprocmask(SIG_BLOCK, &block_set, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("Blocked SIGINT.\n");
    printf("Press Ctrl+C to create pending signal.\n");

    while (1) {
        if (sigpending(&pending_set) == -1) {
            perror("sigpending");
            return 1;
        }

        int result = sigismember(&pending_set, SIGINT);

        if (result == -1) {
            perror("sigismember");
            return 1;
        }

        if (result == 1) {
            printf("Found pending SIGINT. Quitting!\n");
            break;
        }

        sleep(1);
    }

    return 0;
}

