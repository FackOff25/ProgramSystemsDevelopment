#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>

#define SRV_HOST "127.0.0.1"
#define SRV_PORT 8000
#define CLNT_PORT 8001
#define BUF_SIZE 64
#define TXT_ANSW "I am your client\n"


int main (int argc, char** argv) {
    int cl_port = CLNT_PORT;
    int sv_port = CLNT_PORT;
    char* sv_addr = SRV_HOST;
    if(argc >= 4){
        cl_port = atoi(argv[1]);
        sv_addr = argv[2];
        sv_port = atoi(argv[3]);
    }

    int sock;
    int from_len;
    char buf[BUF_SIZE];
    struct hostent *hp;

    sock = socket (AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
 
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(sv_port);

    printf("Connecting... ");
    if(connect (sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0){
        printf("failed\n");
        exit(0);
    }
    printf("connected!\n");
    while(1){
        printf("Your turn: ");
        scanf("%s",buf);
        send(sock, buf, BUF_SIZE, 0);
        recv(sock, buf, BUF_SIZE, 0);
        printf("Enemy's answer is %s\n", buf);

        recv(sock, buf, BUF_SIZE, 0);
        printf("Enemy's turn is %s\n", buf);
        printf("Your answer: ");
        scanf("%s",buf);
        send(sock, buf, BUF_SIZE, 0);
    }
    close (sock);
}