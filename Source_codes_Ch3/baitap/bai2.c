#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>

void convert_ipv4(const char *ip_string){
    struct in_addr ipv4_addr;

    if(inet_pton(AF_INET, ip_string, &ipv4_addr) == 1){
        printf("Successfully converted to binary IP Address: %s\n", ip_string);
    } else{
        printf("Convert failed\n");
        exit(EXIT_FAILURE);
    }
    // 16 byte
    char ip_str_convered[INET_ADDRSTRLEN];

    if(inet_ntop(AF_INET, &ipv4_addr, ip_str_convered, INET_ADDRSTRLEN)){
        printf("Successfully converted from binary to string: %s\n", ip_str_convered);
    } else{
        printf("Convert failed\n");
        exit(EXIT_FAILURE);
    }
}

void convert_ipv6(const char *ip_string){
    struct in_addr ipv6_addr;

    if(inet_pton(AF_INET6, ip_string, &ipv6_addr) == 1){
        printf("Successfully converted to binary IP Address: %s\n", ip_string);
    } else{
        printf("Convert failed\n");
        exit(EXIT_FAILURE);
    }
    // 46
    char ip_str_convered[INET6_ADDRSTRLEN];

    if(inet_ntop(AF_INET6, &ipv6_addr, ip_str_convered, INET6_ADDRSTRLEN)){
        printf("Successfully converted from binary to string: %s\n", ip_str_convered);
    } else{
        printf("Convert failed\n");
        exit(EXIT_FAILURE);
    }
}

int main(){
    char ipv4_str[INET_ADDRSTRLEN];
    char ipv6_str[INET6_ADDRSTRLEN];
    
    printf("Enter IPv4 address:\n");
    scanf("%s", ipv4_str);

    printf("Convert IPv4:\n");
    convert_ipv4(ipv4_str);

    printf("Enter IPv6 address:\n");
    scanf("%s", ipv6_str);

    printf("Convert IPv6:\n");
    convert_ipv6(ipv6_str);

    return 0;
}
