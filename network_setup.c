#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXLINE 8192
#define LISTENQ 1024

void addrinfo_init(struct addrinfo **currAddrInfo, char *hostname, char *port, int flags)
{
    struct addrinfo hints;
    int getaddrerror;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = flags;
    if ((getaddrerror = getaddrinfo(hostname, port, &hints, currAddrInfo)) != 0)
    {
        fprintf(stderr, "Error getting Exploit Server info: %s\n", gai_strerror(getaddrerror));
        freeaddrinfo(*currAddrInfo);
        exit(1);
    }
}

int host_socketfd(char *expServIP, char *expServPort)
{
    struct addrinfo *exploitServer, *itr;
    int hSocketFD;
    addrinfo_init(&exploitServer, expServIP, expServPort, 0);
    for (itr = exploitServer; itr != NULL; itr = itr->ai_next)
    {
        if ((hSocketFD = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol)) < 0)
        {
            continue;
        }
        int flags = NI_NUMERICHOST | NI_NUMERICSERV;
        char expIP[MAXLINE], expPort[MAXLINE];
        getnameinfo(itr->ai_addr, itr->ai_addrlen, expIP, MAXLINE, expPort, MAXLINE, flags);
        printf("\nConnecting to server %s at port %s\n", expIP, expPort);
        if (connect(hSocketFD, itr->ai_addr, itr->ai_addrlen) != -1)
        {
            printf("Connected to server %s at port %s\n", expIP, expPort);
            break;
        }
        close(hSocketFD);
    }
    freeaddrinfo(exploitServer);
    if (!itr)
        return -1;
    else
        return hSocketFD;
}

int exploit_serverfd(char *port)
{
    struct addrinfo *exploitServer, *itr;
    int listenfd;
    addrinfo_init(&exploitServer, NULL, port, AI_PASSIVE | AI_NUMERICSERV);
    for (itr = exploitServer; itr != NULL; itr = itr->ai_next)
    {
        if ((listenfd = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol)) < 0)
        {
            continue;
        }
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)1, sizeof(int));
        int flags = NI_NUMERICHOST | NI_NUMERICSERV;
        char expIP[MAXLINE], expPort[MAXLINE];
        getnameinfo(itr->ai_addr, itr->ai_addrlen, expIP, MAXLINE, expPort, MAXLINE, flags);
        printf("\nStarting server %s at port %s\n", expIP, expPort);
        if (bind(listenfd, itr->ai_addr, itr->ai_addrlen) == 0)
        {
            break;
        }
        close(listenfd);
    }
    freeaddrinfo(exploitServer);
    if (!itr)
        return -1;
    if (listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);
        return -1;
    }
    return listenfd;
}