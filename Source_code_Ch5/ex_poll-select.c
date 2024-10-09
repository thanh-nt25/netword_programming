#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <poll.h>
#include <time.h>

void use_select() {
    printf("\nUsing select()...\n");
    
    fd_set readfds;
    struct timeval timeout;
    int retval;
    
    // Clear the set and add stdin (fd 0) to the set
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    // Set timeout to 5 seconds
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    // Wait for input on stdin
    retval = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    
    if (retval == -1) {
        perror("select()");
    } else if (retval) {
        printf("Data is available to read using select().\n");
    } else {
        printf("No data within five seconds using select().\n");
    }
}

void use_poll() {
    printf("\nUsing poll()...\n");
    
    struct pollfd fds[1];
    int retval;
    
    // Set up the pollfd structure for stdin
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    
    // Wait for input on stdin, timeout is 5000 milliseconds (5 seconds)
    retval = poll(fds, 1, 5000);
    
    if (retval == -1) {
        perror("poll()");
    } else if (retval) {
        if (fds[0].revents & POLLIN) {
            printf("Data is available to read using poll().\n");
        }
    } else {
        printf("No data within five seconds using poll().\n");
    }
}

int main() {
    // Demonstrating select()
    use_select();
    
    // Demonstrating poll()
    use_poll();
    
    return 0;
}
