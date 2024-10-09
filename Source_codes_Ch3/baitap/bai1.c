#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(){
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    // set to IPv4
    server_addr.sin_family = AF_INET;

    server_addr.sin_port = htons(8080);

    if(inet_pton(AF_INET, "192.168.1.1", &server_addr.sin_addr) <= 0){
        printf("Invalid\n");
        return -1;
    }

    printf("Socket init:\n");
    printf("Family: IPv4\n");
    printf("Port: %d\n", ntohs(server_addr.sin_port));
    printf("IP Address: 192.168.1.1\n");

    return 0;
}


