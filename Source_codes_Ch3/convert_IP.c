#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main() {
    const char *ip_str = "192.168.1.1";  // Example IP address in string format
    struct in_addr ip_addr;              // Structure to hold the IP address in binary form

    // Convert the IP address from string to binary form using inet_aton
    if (inet_aton(ip_str, &ip_addr)) {
        printf("inet_aton: Successfully converted IP address: %s\n", ip_str);
    } else {
        printf("inet_aton: Failed to convert IP address: %s\n", ip_str);
        exit(EXIT_FAILURE);
    }

    // Convert the binary IP address back to string form using inet_ntoa
    char *ip_str_converted = inet_ntoa(ip_addr);
    if (ip_str_converted) {
        printf("inet_ntoa: Converted back to string IP address: %s\n", ip_str_converted);
    } else {
        printf("inet_ntoa: Failed to convert IP address back to string\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
