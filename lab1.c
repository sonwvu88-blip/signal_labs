
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int signo)
{
    (void)signo;

    const char msg[] = "Hello World\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}
int main(void)
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    printf("Programme is running.\n");
    printf("Press Ctrl+C to print Hello World.\n");

    while (1) {
        pause();
    }

    return 0;
}
