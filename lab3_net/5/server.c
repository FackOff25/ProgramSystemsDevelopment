#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "protocol.h"

#define PORT 8000
#define BUF_SIZE 64

int readConn(int connection, char *buf, Message *mes, CODINGS defCoding)
{
    read(connection, buf, BUF_SIZE);
    printf("%s\n", buf);
    parseMessage(buf, BUF_SIZE, mes);
    if (mes->coding != defCoding)
    {
        printf("Coding error\n");
        makeCodingError(buf);
        write(connection, buf, BUF_SIZE);
        return 1;
    }
    return 0;
}

int scanAndSendConn(int connection, char *buf, Message *mes, CODINGS defCoding)
{
    mes->coding = defCoding;
    scanf("%s", mes->message);
    makeMessage(mes, BUF_SIZE, buf);
    write(connection, buf, BUF_SIZE);
    return 0;
}

int main(int argc, char **argv)
{
    int port = PORT;
    if (argc >= 2)
    {
        port = atoi(argv[1]);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
    {
        printf("Didn't manage to bind port %d\n", port);
        return 0;
    }
    if (listen(sock, 10) != 0)
    {
        printf("Didn't manage to listen port %d\n", port);
        return 0;
    }
    char buf[BUF_SIZE], pars[BUF_SIZE];
    Message mes;
    mes.message = pars;
    CODINGS coding;

    printf("Server listens port %d\n", port);
    int connfd = accept(sock, (struct sockaddr_in *)NULL, NULL);
    printf("Got connection!\n");

    read(connfd, buf, BUF_SIZE);
    parseMessage(buf, BUF_SIZE, &mes);

    coding = mes.coding;

    printf("Enemy's turn is %s\n", mes.message);
    printf("Your answer: ");
    scanf("%s", mes.message);
    makeMessage(&mes, BUF_SIZE, buf);
    write(connfd, buf, BUF_SIZE);

    while (1)
    {
        printf("Your turn: ");
        scanAndSendConn(connfd, buf, &mes, coding);

        if (readConn(connfd, buf, &mes, coding) == 1)
            break;
        printf("Enemy's answer is %s\n", buf);

        if (readConn(connfd, buf, &mes, coding) == 1)
            break;
        printf("Enemy's turn is %s\n", mes.message);

        printf("Your answer: ");
        scanAndSendConn(connfd, buf, &mes, coding);
    }
    close(connfd);

    close(sock);
}