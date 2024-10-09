#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

volatile sig_atomic_t got_signal = 0;

void signal_handler(int sig) {
    got_signal = 1;
    printf("Caught signal %d\n", sig);
}

void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}

int main() {
    fd_set readfds;
    struct timespec timeout;
    sigset_t block_mask, orig_mask;
    
    // Setup signal handler for SIGINT
    setup_signal_handler();
    
    // Block SIGINT
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGINT);
    sigprocmask(SIG_BLOCK, &block_mask, &orig_mask);

    printf("SIGINT blocked, waiting for input...\n");

    // Use pselect with signal mask modification
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    // timeout.tv_sec = 20;   // 20 seconds timeout
    // timeout.tv_nsec = 0;
    
    printf("Starting the pselect ... \n");
    // pselect atomically sets the original mask before waiting
    // int ready = pselect(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL, &orig_mask);
    int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL);
    

    if (ready == -1) {
        if (errno == EINTR) {
            printf("pselect() was interrupted by a signal.\n");
        } else {
            perror("pselect");
        }
    } else if (ready == 0) {
        printf("pselect() timed out.\n");
    } else {
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char buffer[128];
            read(STDIN_FILENO, buffer, sizeof(buffer));
            printf("Input received: %s\n", buffer);
        }
    }
    
    // Restore the original signal mask
    sigprocmask(SIG_SETMASK, &orig_mask, NULL);

    printf("Exiting...\n");
    return 0;
}
