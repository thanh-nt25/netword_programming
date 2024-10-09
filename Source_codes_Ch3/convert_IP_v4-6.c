#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

void convert_ipv4(const char *ip_str) {
    struct in_addr ipv4_addr;    // Structure to hold the IPv4 address

    // Convert the IPv4 address from string to binary form
    if (inet_pton(AF_INET, ip_str, &ipv4_addr) == 1) {
        printf("inet_pton (IPv4): Successfully converted IP address: %s\n", ip_str);
    } else {
        printf("inet_pton (IPv4): Failed to convert IP address: %s\n", ip_str);
        exit(EXIT_FAILURE);
    }

    char ip_str_converted[INET_ADDRSTRLEN];  // Buffer to hold the converted IP address back to string

    // Convert the binary IPv4 address back to string form
    if (inet_ntop(AF_INET, &ipv4_addr, ip_str_converted, INET_ADDRSTRLEN)) {
        printf("inet_ntop (IPv4): Converted back to string IP address: %s\n", ip_str_converted);
    } else {
        printf("inet_ntop (IPv4): Failed to convert IP address back to string\n");
        exit(EXIT_FAILURE);
    }
}

void convert_ipv6(const char *ip_str) {
    struct in6_addr ipv6_addr;   // Structure to hold the IPv6 address

    // Convert the IPv6 address from string to binary form
    if (inet_pton(AF_INET6, ip_str, &ipv6_addr) == 1) {
        printf("inet_pton (IPv6): Successfully converted IP address: %s\n", ip_str);
    } else {
        printf("inet_pton (IPv6): Failed to convert IP address: %s\n", ip_str);
        exit(EXIT_FAILURE);
    }

    char ip_str_converted[INET6_ADDRSTRLEN]; // Buffer to hold the converted IP address back to string

    // Convert the binary IPv6 address back to string form
    if (inet_ntop(AF_INET6, &ipv6_addr, ip_str_converted, INET6_ADDRSTRLEN)) {
        printf("inet_ntop (IPv6): Converted back to string IP address: %s\n", ip_str_converted);
    } else {
        printf("inet_ntop (IPv6): Failed to convert IP address back to string\n");
        exit(EXIT_FAILURE);
    }
}

int main() {
    const char *ipv4_str = "192.168.1.1";         // Example IPv4 address
    const char *ipv6_str = "2001:0db8::1";        // Example IPv6 address

    // Demonstrate IPv4 conversion
    printf("IPv4 Conversion:\n");
    convert_ipv4(ipv4_str);

    printf("\n");

    // Demonstrate IPv6 conversion
    printf("IPv6 Conversion:\n");
    convert_ipv6(ipv6_str);

    return 0;
}
