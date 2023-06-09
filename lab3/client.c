#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "protocol.h"
#include "marine.h"

#define SRV_HOST "127.0.0.1"
#define SRV_PORT 8000
#define BUF_SIZE 64

int readConn(int connection, char *buf, Message *mes, CODINGS defCoding)
{
    recv(connection, buf, BUF_SIZE, 0);
    parseMessage(buf, BUF_SIZE, mes);
    if (mes->coding != defCoding)
    {
        printf("Coding error\n");
        makeErrorMessage(buf);
        send(connection, buf, BUF_SIZE, 0);
        return 1;
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
        fflush(stdin);
        scanMessage(mes, defCoding);
    }
    return sendMessage(connection, buf, mes);
}


int getCodingFromUser(CODINGS *coding)
{
    int codeBuf = 0;
    printf("Which encoding will you use (1-latin, 2-cyrillic): ");
    scanf("%d", &codeBuf);
    while (codeBuf != 1 && codeBuf != 2)
    {
        printf("Wrong input, try again: ");
        scanf("%d", &codeBuf);
    }
    *coding = codeBuf;
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

int main(int argc, char **argv)
{
    int sv_port = SRV_PORT;
    char *sv_addr = SRV_HOST;
    if (argc == 2)
    {
        sv_port = atoi(argv[1]);
    }
    else if (argc >= 3)
    {
        sv_addr = argv[1];
        sv_port = atoi(argv[2]);
    }

    int sock;
    int from_len;
    char buf[BUF_SIZE], pars[BUF_SIZE];
    Message mes;
    mes.message = pars;
    CODINGS coding;
    struct hostent *hp;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(sv_port);

    printf("Connecting... ");
    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("failed\n");
        exit(0);
    }
    printf("connected!\n");

    getCodingFromUser(&coding);

    while (1)
    {
        if (shootPhase(sock, buf, &mes, coding) == -1)
            break;

        if (recievePhase(sock, buf, &mes, coding) == -1)
            break;
    }
    printf("Something on the other side went wrong\n");
    close(sock);
}