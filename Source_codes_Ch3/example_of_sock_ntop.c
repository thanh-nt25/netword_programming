#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_ADDR_STRLEN 128

// Custom sock_ntop function to convert a socket address into a string (IP and port)
char *sock_ntop(const struct sockaddr *sa, socklen_t salen) {
    static char str[MAX_ADDR_STRLEN];   // Buffer to hold the string representation
    char portstr[8];                    // Buffer to hold the port as a string

    // Check if the address is IPv4
    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *) sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    }
    // Check if the address is IPv6
    else if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin6->sin6_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    } else {
        return NULL;  // Unsupported address family
    }
}

int main() {
    struct sockaddr_in ipv4_addr;
    struct sockaddr_in6 ipv6_addr;
    
    // Set up an IPv4 socket address (192.168.1.1:8080)
    ipv4_addr.sin_family = AF_INET;
    ipv4_addr.sin_port = htons(8080);  // Set port to 8080
    inet_pton(AF_INET, "192.168.1.1", &ipv4_addr.sin_addr); // Convert string IP to binary form

    // Set up an IPv6 socket address (2001:db8::1:8080)
    ipv6_addr.sin6_family = AF_INET6;
    ipv6_addr.sin6_port = htons(8080);  // Set port to 8080
    inet_pton(AF_INET6, "2001:db8::1", &ipv6_addr.sin6_addr); // Convert string IP to binary form

    // Convert and print IPv4 address using sock_ntop
    printf("IPv4 Address: %s\n", sock_ntop((struct sockaddr *) &ipv4_addr, sizeof(ipv4_addr)));

    // Convert and print IPv6 address using sock_ntop
    printf("IPv6 Address: %s\n", sock_ntop((struct sockaddr *) &ipv6_addr, sizeof(ipv6_addr)));

    return 0;
}
