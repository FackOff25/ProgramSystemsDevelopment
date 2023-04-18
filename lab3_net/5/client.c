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

#define SRV_HOST "127.0.0.1"
#define SRV_PORT 8000
#define CLNT_PORT 8001
#define BUF_SIZE 64
#define TXT_ANSW "I am your client\n"

int readConn(int connection, char *buf, Message *mes, CODINGS defCoding)
{
    recv(connection, buf, BUF_SIZE, 0);
    parseMessage(buf, BUF_SIZE, mes);
    if (mes->coding != defCoding)
    {
        printf("Coding error\n");
        makeCodingError(buf);
        send(connection, buf, BUF_SIZE, 0);
        return 1;
    }
    return 0;
}

int scanAndSendConn(int connection, char *buf, Message *mes, CODINGS defCoding)
{
    mes->coding = defCoding;
    scanf("%s", mes->message);
    makeMessage(mes, BUF_SIZE, buf);
    send(connection, buf, BUF_SIZE, 0);
    return 0;
}

int getCodingFromUser(CODINGS* coding){
    int codeBuf = 0;
    printf("Which encoding will you use (1-latin, 2-cyrillic): ");
    scanf("%d", &codeBuf);
    while(codeBuf != 1 && codeBuf != 2){
        printf("Wrong input, try again: ");
        scanf("%d", &codeBuf);
    }
    *coding = codeBuf;
}

int main (int argc, char** argv) {
    int cl_port = CLNT_PORT;
    int sv_port = SRV_PORT;
    char* sv_addr = SRV_HOST;
    if(argc >= 4){
        cl_port = atoi(argv[1]);
        sv_addr = argv[2];
        sv_port = atoi(argv[3]);
    }

    int sock;
    int from_len;
    char buf[BUF_SIZE], pars[BUF_SIZE];
    Message mes;
    mes.message = pars;
    CODINGS coding;
    struct hostent *hp;

    sock = socket (AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
 
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(sv_port);

    printf("Connecting... ");
    if(connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0){
        printf("failed\n");
        exit(0);
    }
    printf("connected!\n");

    getCodingFromUser(&coding);

    while(1){
        printf("Your turn: ");
        scanAndSendConn(sock, buf, &mes, coding);

        if (readConn(sock, buf, &mes, coding) == 1)
            break;
        printf("Enemy's answer is %s\n", buf);

        if (readConn(sock, buf, &mes, coding) == 1)
            break;
        printf("Enemy's turn is %s\n", mes.message);

        printf("Your answer: ");
        scanAndSendConn(sock, buf, &mes, coding);
    }
    close (sock);
}