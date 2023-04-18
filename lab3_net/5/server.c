#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT 8000
#define BUF_SIZE 256

int main(int argc, char** argv){
    int port = PORT;
    if(argc >= 2){
        port = atoi(argv[1]);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = port; 

    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0){
        printf("Didn't manage to bind port %d\n", port);
        return 0;
    }
    if (listen(sock, 10) != 0){
        printf("Didn't manage to listen port %d\n", port);
        return 0;
    }
    char buf[BUF_SIZE];
    printf("Server listens port %d\n", port);

    int connfd = accept(sock, (struct sockaddr*)NULL, NULL); 
    while(1){
        recv(sock, buf, BUF_SIZE, 0);
        printf("Enemy's turn is %s\n", buf);
        printf("Your answer: ");
        scanf("%s",buf);
        write(sock, buf, BUF_SIZE);
        printf("Your turn: ");
        scanf("%s",buf);
        write(sock, buf, BUF_SIZE);
        recv(sock, buf, BUF_SIZE, 0);
        printf("Enemy's answer is %s\n", buf);
    }
    close(connfd);
    
    close(sock);
}