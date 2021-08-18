#define MAX_BUFFER_SIZE 1024
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>


unsigned int araay_printable[95] = {0};
bool last_time = 0, is_exist_client = 0;


unsigned int read_(int connfd, char* buff)
{
    unsigned int n = 0,len;

    while (n != sizeof(len))
    {
        n += read(connfd, &len + n, sizeof(len) - n);
    }

    n = 0;
    while (n != len)
    {
        n += read(connfd, buff + n, MAX_BUFFER_SIZE - 1);
    }
    return len;
}


void sent_(int connfd, int nsent,void* count_of_printable)
{
        unsigned int total_sent = 0, not_sent = sizeof(count_of_printable);

        while(not_sent > 0)
        {
            nsent = write(connfd,count_of_printable + total_sent,not_sent);
            assert(nsent >= 0);

            total_sent += nsent;
            not_sent -= nsent;
        }
}


size_t araay_of_num_printable(unsigned int len, char* buff)
{
    size_t count_of_printable=0;
    for (int i = 0; i < len; ++i)
    {
        if (31<buff[i] && buff[i] <127 )
        {
            count_of_printable++;
            araay_printable[buff[i] - 32] += 1;
        }
    }
    return count_of_printable;
}


void print_printable()
{
    unsigned char i;
    for (i = 0; i <95; ++i)
    {
        printf("char ’%c’ : %u times\n", i + 32, araay_printable[i]);
    }

    exit(0);
}

void exit_()
{
    last_time = true;
    if (is_exist_client)
    {
        print_printable();
    }
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = exit_;
    sigaction(SIGINT, &sa, NULL);

    size_t count_of_printable = 0;
    int nsent = -1,listenfd = -1, connfd = -1;
    char buff[MAX_BUFFER_SIZE];

    struct sockaddr_in server_addr,my_addr,peer_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in );

    listenfd = socket( AF_INET, SOCK_STREAM, 0 );
    memset(&server_addr, 0, addr_size );

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));


    if(bind(listenfd, (struct sockaddr*) &server_addr, addr_size ) != 0)
    {
        printf("\n Error : Bind Failed. %s \n", strerror(errno));
        return 1;
    }

    if(listen( listenfd, 10 ) != 0)
    {
        printf("\n Error : Listen Failed. %s \n", strerror(errno));
        return 1;
    }

    while(true)
    {

        if (last_time == true)
        {
            print_printable();
        }

        connfd = accept( listenfd,(struct sockaddr*) &peer_addr,&addr_size);
        is_exist_client = true;

        if(connfd < 0)
        {
            printf("\n Error : Accept Failed. %s \n", strerror(errno));
            if (strcmp(strerror(errno), "ETIMEDOUT") != 0 || strcmp(strerror(errno), " ECONNRESET") != 0 || strcmp(strerror(errno), "EPIPE") != 0)
            {
                continue;
            }
            else
            {
                exit(1);
            }
        }

        if (setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &is_exist_client, sizeof(is_exist_client)) == -1)
        {
            printf("setsockopt fail");
        }


        unsigned int len;
        len=read_(connfd,buff);
        count_of_printable=araay_of_num_printable(len,buff);

        getsockname(connfd, (struct sockaddr*) &my_addr,   &addr_size);
        getpeername(connfd, (struct sockaddr*) &peer_addr, &addr_size);

        sent_(connfd,nsent,&count_of_printable);

        close(connfd);
        is_exist_client = false;
    }
}
