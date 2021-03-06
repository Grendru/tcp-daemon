#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sys/poll.h>
#include <string.h>

#define PORT 3325

using namespace std;


char* readfile(char path_to_file[], int* datalen)
{

    std::ifstream is (path_to_file, std::ifstream::binary);
    if (is) {
        is.seekg (0, is.end);
        int length = is.tellg();
        is.seekg (0, is.beg);

        char *buffer = new char [length];
        is.read (buffer,length);

        if (!is)
            printf("error: only %ld could be read\n", is.gcount());
        is.close();
        *datalen = length;
        return buffer;
    }
    else
        printf("File not open\n");
    return NULL;
}
int get_sock(int ip, char *ipchar, int port)
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip;
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(2);
    }
    return sock;
}

int send_data(int sock, char data[], char filename[], int datalen)
{
    int filenamelen = 0;
    for (char *c = filename; *c ; c++)
    {
        filenamelen++;
    }
    send(sock, &filenamelen, sizeof(int), 0);
    send(sock, &datalen, sizeof(int), 0);
    send(sock, filename, filenamelen, 0);
    send(sock, data, datalen, 0);
    return 0;
}
int main(int argc, char* argv[])
{
    char *data;
    int datalen;
    int ip, port;
    if (argc >= 4)
    {
        if ((data = readfile(argv[3], &datalen)) == NULL )
            return 0;
        port=atoi(argv[2]);
        if ((port < 0 || port > 65535 ) || inet_pton(AF_INET, argv[1], &ip) != 1)
        {
            printf("Invalid ip or port\n");
            return 1;
        }
    }
    else
    {
        printf("Use \"./client <ip> <port> <path_to_file>\"\n");
        return 0;
    }
    int sock = get_sock(ip,argv[1], port);
    char *filename, *cur_str;
    cur_str = filename = strtok(argv[3], "/\\");
    while (cur_str != NULL)
    {
        filename = cur_str;
        cur_str = strtok (NULL, "/\\");
    }
    send_data(sock, data, filename, datalen);
    return 0;
}