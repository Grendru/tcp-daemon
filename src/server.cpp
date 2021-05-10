#include <stdlib.h>
#include <stdio.h>
 #include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <fstream>  
#include <signal.h>
#include <wait.h>

#define PORT 3325
#define MAX_CLIENTS 128

using namespace std;
bool terminate_ = 0;
bool sighup = 0;

void signal_handler(int sig)
{   
    switch(sig)
	{
		case SIGHUP:
			sighup = 1;
		case SIGTERM:
            terminate_ = 1;
			break;
	}
}

int write_to_file(char filename[], char data[], int datalen)
{
    std::ofstream outfile(filename,std::ofstream::binary);
    outfile.write(data, datalen);
    outfile.close();
    return 0;
}

int daemon(int port)
{
    int listener;
    struct sockaddr_in addr;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }

    int enable = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    fcntl(listener, F_SETFL, O_NONBLOCK);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }
    listen(listener, 1);

    if (signal(SIGTERM, signal_handler) == SIG_ERR)
    {
        exit(EXIT_FAILURE);
    }

    if (signal(SIGHUP, signal_handler) == SIG_ERR)
    {
        exit(EXIT_FAILURE);
    }

    struct pollfd fd[MAX_CLIENTS];
    int numclients = 0; 
    fd[0].fd = listener;
    fd[0].events = POLLIN;
    for (int i = 1; i < MAX_CLIENTS; i++)
        fd[i].fd = -1;
    while(!terminate_)
    {
        int ret = poll( fd, numclients + 1, -1 );
        if ( ret == -1 )
            {
                printf("Error\n");
                break;
            }
        else if ( ret == 0 )
            {
                printf("Time out\n");
                break;
            }
        else
        {
            if ( fd[0].revents & POLLIN )
                {
                    fd[0].revents = 0;
                    int sock = accept(listener, NULL, NULL);
                    if(sock < 0)
                    {
                        perror("accept");
                        exit(3);
                    }
                    int i;
                    for(i = 1; i < MAX_CLIENTS; i++)
                    {
                        if(fd[i].fd < 0)
                        {
                            fd[i].fd = sock;
                            fd[i].events = POLLIN;
                            break;
                        }
                    }
                    if(i == MAX_CLIENTS)
                    {
                        continue;
                    }
                    if (i > numclients)
                        numclients = i;
                    if (--ret <= 0)
                        continue;
                }
                for (int i = 0; i <= numclients; i++)
                {
                    if (fd[i].fd < 0)
                        continue;
                    if(fd[i].revents & POLLIN)
                    {
                        int recdatalen;
                        int filenamelen, datalen;
                        recdatalen = read(fd[i].fd, &filenamelen, sizeof(int));
                        char *filename = new char[filenamelen];
                        recdatalen = read(fd[i].fd, &datalen, sizeof(int));
                        char *data = new char[datalen];
                        recdatalen = read(fd[i].fd, filename, filenamelen);
                        for(int j = 0; j < datalen; j +=recdatalen)
                        {
                            recdatalen = read(fd[i].fd, data + j, 32768);
                        }
                        close(fd[i].fd);
                        fd[i].fd = -1;
                        write_to_file(filename, data, datalen);
                        delete[] data;
                        delete[] filename;
                    }
                }

        }
    }
    if (terminate_)
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (fd[i].fd != -1)
                    close(fd[i].fd);
        }
        if (sighup)
        {
            terminate_ = 0;
            sighup = 0;
            daemon(port);
        }
        
    }
    return 0;
}

int main(int argc, char *argv[])
{
    struct stat   buffer;
    if (argc <= 2)
    {
        printf("Use \"./server <port> <absolute_path_to_folder>\"\n");
        return(0);
    }
    else if (stat (argv[2], &buffer) != 0)
    {
        printf("Folder not exists\n");
        return(0);
    }
    else if (atoi(argv[1]) < 0 || atoi(argv[1]) > 65535 )
    {
        printf("invalid port\n");
        return 0;
    }

    int port = atoi(argv[1]);
    pid_t parpid, sid;
    
    parpid = fork(); 
    if(parpid < 0) {
        exit(1);
    } else if(parpid != 0) {
        exit(0);
    } 
    umask(0);
    sid = setsid();
    if(sid < 0) {
        exit(1);
    }
    if((chdir(argv[2])) < 0) {
        exit(1);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    daemon(port);
    return 0;
}