#ifndef _MFTP_H
#define _MFTP_H

#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#define MY_PORT_NUMBER 49999
#define TIMEOUT 60

struct sockaddr_in servAddr;
struct hostent* hostEntry;
struct in_addr **pptr;
struct sockaddr_in servAdder;
struct sockaddr_in clientAddr;
struct sockaddr_in dataAddr;

int readNet(int fd, char *buffer){
     int n = 0;
     while (true){
        n = read(fd, buffer, 1);
        if (n < 0){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            return -1;
        }
        if (*buffer == '\n') break;
        buffer++;
        if(n == 0){
            return -1;
        }
    }
    *(buffer + 1) = '\0';
    return n;
}

void writeNet(int fd, char *buffer){
    int n = write(fd, buffer, sizeof(char) * strlen(buffer));
    if (n < 0){
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return;
    }
}

void writeNetFile(int fd, int fileDP, char* buffer){
    while (true){
        int n = read(fileDP, buffer, 512);
        if (n < 0){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            return;
        }
        if (n == 0) break;
        write(fd, buffer, n);
    }
}

void readNetFile(int fd, int fileDP, char* buffer){
    while (true){
        int n = read(fd, buffer, 512);
        if (n < 0){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            return;
        }
        if (n == 0) break;
        write(fileDP, buffer, n);
    }
}

void removeNewLine(char* buffer){
    int i = 0;
    while (buffer[i] != '\n'){
        i++;
    }
    buffer[i] = '\0';
}

void sendAcknowledgement(int fd, char *buffer){
    strcpy(buffer, "A\n");
    writeNet(fd, buffer);
}

void sendError(int fd, char *buffer){
    buffer[0] = 'E';
    sprintf(buffer + 1, "Error: %s\n", strerror(errno));
    writeNet(fd, buffer);
}

void parse(char *buffer, char *command, char *pathname){
    int n = sscanf(buffer, "%s %[^\n]", command, pathname);
    if (n == EOF){
        return;
    }
}

void parsePathname(char *buf, char *pathname){
    int i;
    int lastSlash = 0;
    for (i = 0; buf[i]; i++){
        if (buf[i] == '/'){
            lastSlash = i;
        }
    }
    if (lastSlash == 0) return;
    strcpy(pathname, buf + lastSlash + 1);
}

void readNetVersion2(int fd, char *buffer){
    while (true){
        int n = read(fd, buffer, 1);
        if (n < 0){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            return;
        }
        if (n == 0) break;
        buffer++;
    }
    if (*buffer == '\n'){
        *(buffer++) = '\0';
    }
}

void printFile(int fd, char* buffer){
    while (true){
        int n = read(fd, buffer, 512);
        if (n < 0){
            fprintf(stderr, "Error %s\n", strerror(errno));
            return;
        }
        if (n == 0) break;
        write(1, buffer, n);
    }
}

#endif
