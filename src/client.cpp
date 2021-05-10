#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
int get_sock()
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
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
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
    if (argc >= 2)
    {
        if ((data = readfile(argv[1], &datalen)) == NULL )
            return 0;

    }
    else
    {
        printf("Use \"./client <path_to_file>\"\n");
        return 0;
    }
    int sock = get_sock();
    char *filename, *cur_str;
    cur_str = filename = strtok(argv[1], "/\\");
    while (cur_str != NULL)
    {
        filename = cur_str;
        cur_str = strtok (NULL, "/\\");
    }
    send_data(sock, data, filename, datalen);
    return 0;
}