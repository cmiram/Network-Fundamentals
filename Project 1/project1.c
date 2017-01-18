#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdbool.h>

bool received_status(char *msg);
char* parse_and_solve_msg(char *a, char *math_func, char *b);

int main(int argc, char *argv[]) {
    
    int port = 27993;
    char *host;
    char *servIP;
    char *nuid;
    if(argc < 3 || argc > 6) {
        printf("Please enter arguments of the form:\n");
        printf("./client <-p port> <-s> [hostname] [NEU ID]");
        return -1;
    }
    else if (argv[1] == "-p") {
        port = argv[2];
        host = argv[3];
        servIP = argv[3];
        nuid = argv[4];
    }
    else {
        host = argv[1];
        servIP = argv[1];
        nuid = argv[2];
    }
    
    struct hostent *ip;
    int sock;
    struct sockaddr_in server_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        printf("error creating socket %d\n", sock);
    }
    
    ip = gethostbyname(host);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(servIP);
    server_addr.sin_port = htons(port);
    
    if(connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("connect - %s\n", strerror(errno));
    }
    
    char *hello_suffix = "\n";
    char *buffer_sent[256];
    strcpy(buffer_sent, "cs5700spring2017 HELLO ");
    strcat(buffer_sent, nuid);
    strcat(buffer_sent, hello_suffix);
    int buff_len = strlen(buffer_sent);
    if(send(sock, buffer_sent, buff_len, 0) < 0) {
        printf("send - %s\n", strerror(errno));
    }
    
    char buffer_rec[256];
    if(recv(sock, buffer_rec, 255, 0) < 0) {
        printf("recv failed - %s", strerror(errno));
    }
    
    char *tok;
    char *sol;
    while(received_status(buffer_rec)) {
        tok = strtok(buffer_rec, ' ');
        strcpy(buffer_sent, tok);
        strcat(buffer_sent, ' ');
        tok = strtok(NULL, ' '); //skip STATUS token
        sol = parse_and_solve_msg(strtok(NULL, ' ' ), strtok(NULL, ' ' ), strtok(NULL, ' ' ));
        strcat(buffer_sent, sol);
        strcat(buffer_sent, '\n');
        
        buff_len = strlen(buffer_sent);
        if(send(sock, buffer_sent, buff_len, 0) < 0) {
            printf("sending solution failed - %s", strerror(errno));
            return -1;
        }
        
        if(recv(sock, buffer_rec, 255, 0) < 0) {
            printf("recveiving new response failed - %s", strerror(errno));
        }
    }
    
    tok = strtok(buffer_rec, ' '); //skip first token
    tok = strtok(NULL, ' '); //now token is secret flag
    printf("secret flag: %s", tok);
    close(sock);
    
    return 0;
}

bool received_status(char *msg) {
    int len = strlen(msg);
    char tmp[4];
    memcpy(tmp, msg[len-4], 3);
    tmp[3] = '\0';
    if(strcmp(tmp, "BYE") == 0) {
        return true;
    }
    return false;
}

char* parse_and_solve_msg(char *a, char *math_func, char *b) {
    int in1 = atoi(a);
    int in2 = atoi(b);
    int sol;
    char *sol_str;
    
    switch(*math_func) {
        case '+':
            sol = in1 + in2;
            break;
        case '-':
            sol = in1 - in2;
            break;
        case '*':
            sol = in1 * in2;
            break;
        case '/':
            sol = in1 / in2;
            break;
        default:
            printf("Parsed math function not recognized\n");
            exit(-1);
    }
    
    sprintf(sol_str, "%d", sol);
    return sol_str;
}