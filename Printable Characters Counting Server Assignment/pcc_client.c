#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define MAX_BUFFER_SIZE 1024
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <netdb.h>
unsigned int write2(int sockfd, char* buff)
{
    unsigned int n=0, len = strlen(buff);
    while (n != sizeof(len))
    {
        n += write(sockfd, &len + n, sizeof(len) - n);
    }

    n = 0;
    while (n != len)
    {
        n += write(sockfd, buff + n, len - n);
    }

    return len;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, SIG_IGN);
    if(argc!=4)
    {
        printf("error: There are not enough arguments\n");
        exit(1);
    }
    char *ip = argv[1];
    char *port = argv[2];
    char *file_path = argv[3];
    char recv_buff[MAX_BUFFER_SIZE];
    char buff[MAX_BUFFER_SIZE];
    int  sockfd = -1;
    int num_of_bytes_read =  0;
    size_t nread, num_of_read = 0;
    unsigned int x = 0;

    struct sockaddr_in server_addr, my_addr, peer_addr;
    socklen_t addrsize = sizeof(struct sockaddr_in );


    FILE *file = fopen(file_path, "r");
    if (file)
    {
        while ((nread = fread(buff + num_of_read, 1, sizeof(buff), file)) > 0)
        {
            num_of_read += nread;
            fwrite(buff, 1, nread, file);
            buff[num_of_read] = '\0';
        }
        fclose(file);
    }
    else
    {
        perror("file open failed");
    }

    memset(recv_buff, 0,sizeof(recv_buff));
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        exit(1);
    }

    getsockname(sockfd,(struct sockaddr*) &my_addr,&addrsize);
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if(connect(sockfd,(struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("\n Error : Connect Failed. %s \n", strerror(errno));
        exit(1);
    }

    write2(sockfd, buff);

    getsockname(sockfd, (struct sockaddr*) &my_addr,   &addrsize);
    getpeername(sockfd, (struct sockaddr*) &peer_addr, &addrsize);


    while((num_of_bytes_read = read(sockfd,&x + num_of_bytes_read, sizeof(x))) > 0);

    printf("# of printable characters: %u\n", x);
    close(sockfd);
    return 0;
}

