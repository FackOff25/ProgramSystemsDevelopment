#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "protocol.h"
#include "marine.h"

#define PORT 8000
#define BUF_SIZE 64

int readConn(int connection, char *buf, Message *mes, CODINGS defCoding)
{
    read(connection, buf, BUF_SIZE);
    parseMessage(buf, BUF_SIZE, mes);
    if (mes->coding != defCoding)
    {
        printf("Coding error\n");
        makeErrorMessage(buf);
        write(connection, buf, BUF_SIZE);
        return -1;
    }
    return 0;
}

void scanMessage(Message *mes, CODINGS defCoding){
    mes->coding = defCoding;
    scanf("%s", mes->message);
}

int sendMessage(int connection, char *buf, Message *mes){
    makeMessage(mes, BUF_SIZE, buf);
    write(connection, buf, BUF_SIZE);
    return 0;
}

int scanAndSendConn(int connection, char *buf, Message *mes, CODINGS defCoding)
{
    scanMessage(mes, defCoding);
    return sendMessage(connection, buf, mes);
}

int scanAndSendAnswer(int connection, char *buf, Message *mes, CODINGS defCoding)
{
    scanMessage(mes, defCoding);
    while(getAnswerFromStr(mes->message, defCoding) == WRONG){
        printf("Prohibited answer, try again: ");
        scanMessage(mes, defCoding);
    }
    return sendMessage(connection, buf, mes);
}

int shootTurn(int connection, char *buf, Message *mes, CODINGS coding)
{
    printf("Your turn: ");
    scanAndSendConn(connection, buf, mes, coding);

    if (readConn(connection, buf, mes, coding) == -1)
        return -1;
    printf("Enemy's answer is %s\n", mes->message);
}

int shootPhase(int connection, char *buf, Message *mes, CODINGS coding)
{
    MarineAnswer answer = HIT;
    int res = shootTurn(connection, buf, mes, coding);
    answer = getAnswerFromStr(mes->message, coding);
    while (res != -1 && shouldFireAgain(answer) && answer != WRONG)
    {
        res = shootTurn(connection, buf, mes, coding);
        answer = getAnswerFromStr(mes->message, coding);
    }

    if (answer == WRONG || res == -1)
    {
        return -1;
    }
}

int recieveTurn(int connection, char *buf, Message *mes, CODINGS coding)
{
    printf("Enemy's turn is ");
    if (readConn(connection, buf, mes, coding) == -1)
        return -1;
    printf("%s\n", mes->message);

    printf("Your answer: ");
    scanAndSendAnswer(connection, buf, mes, coding);
}

int recievePhase(int connection, char *buf, Message *mes, CODINGS coding)
{
    MarineAnswer answer = HIT;
    int res = recieveTurn(connection, buf, mes, coding);
    answer = getAnswerFromStr(mes->message, coding);
    while (res != -1 && shouldFireAgain(answer) && answer != WRONG)
    {
        res = recieveTurn(connection, buf, mes, coding);
        answer = getAnswerFromStr(mes->message, coding);
    }

    if (answer == WRONG || res == -1)
    {
        return -1;
    }
}

int firstRecievePhase(int connection, char *buf, Message *mes, CODINGS *coding)
{
    read(connection, buf, BUF_SIZE);
    parseMessage(buf, BUF_SIZE, mes);

    *coding = mes->coding;

    printf("Enemy's turn is %s\n", mes->message);
    printf("Your answer: ");
    scanAndSendAnswer(connection, buf, mes, *coding);

    int res = 0;
    MarineAnswer answer = getAnswerFromStr(mes->message, *coding);
    while (res != -1 && shouldFireAgain(answer) && answer != WRONG)
    {
        res = recieveTurn(connection, buf, mes, *coding);
        answer = getAnswerFromStr(mes->message, *coding);
    }

    if (answer == WRONG || res == -1)
    {
        return -1;
    }
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
    int connfd = accept(sock, NULL, NULL);
    printf("Got connection!\n");

    firstRecievePhase(connfd, buf, &mes, &coding);

    while (1)
    {
        if (shootPhase(connfd, buf, &mes, coding) == -1)
            break;

        if (recievePhase(connfd, buf, &mes, coding) == -1)
            break;
    }
    close(connfd);

    printf("Something on the other side went wrong\n");

    close(sock);
}