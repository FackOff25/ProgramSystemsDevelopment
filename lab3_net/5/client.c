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
    struct sockaddr_in clnt_sin, srv_sin;

    sock = socket (AF_INET, SOCK_STREAM, 0);
    memset ((char *)&clnt_sin, '\0', sizeof(clnt_sin));
    clnt_sin.sin_family = AF_INET;
    clnt_sin.sin_addr.s_addr = INADDR_ANY;
    clnt_sin.sin_port = cl_port;
    bind (sock, (struct sockaddr *)&clnt_sin, sizeof(clnt_sin));

    memset ((char *)&srv_sin, '\0', sizeof(srv_sin));
    hp = gethostbyname (sv_addr);
    srv_sin.sin_family = AF_INET;
    memcpy ((char *)&srv_sin.sin_addr,hp->h_addr_list[0],hp->h_length);
    srv_sin.sin_port = sv_port;

    printf("Connecting... ");
    connect (sock, (struct sockaddr*)&srv_sin, sizeof(srv_sin));
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