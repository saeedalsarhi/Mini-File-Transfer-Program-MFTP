#include "mftp.h"

int createSocket(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0){
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(-1);
    }
    return fd;
}
//Binding
void socketBinding(int fd, struct sockaddr_in *server, int portNumber){
    memset(server, 0, sizeof(struct sockaddr_in));
    server -> sin_family = AF_INET;
    server -> sin_port = htons(portNumber);
    server -> sin_addr.s_addr = htonl(INADDR_ANY);
    if ((bind(fd, (struct sockaddr *) server, sizeof(servAddr)) < 0)){
        perror("Binding faild");
        exit(-1);
    }
}
//get the host name of the client and returns the name
char* getHostName(struct sockaddr_in * client, char* buffer){
    hostEntry = gethostbyaddr(&(client -> sin_addr), sizeof(struct in_addr), AF_INET);
    if (hostEntry == NULL){
        buffer = "Unkown Host";
    }else{
        buffer = hostEntry->h_name;
    }
    return buffer;
}
//Accept client connection
int acceptConnection(int fd, struct sockaddr_in* client){
    int length = sizeof(struct sockaddr_in);
    fd = accept(fd, (struct sockaddr *) client, (socklen_t*) &length);
    if(fd < 0){
        fprintf(stderr,"Error: %s\n", strerror(errno));
        exit(-1);
    }
    return fd;
}

//print the information of the client once connected
void printConnectionInfo(char* buffer){
    buffer = getHostName(&clientAddr, buffer);
    printf("Child %d: Client name is %s\n", getpid(), buffer);
    printf("Child %d: Client IP address is %s\n", getpid(), inet_ntoa(clientAddr.sin_addr));
    printf("Child %d: Accepted Connection from host %s\n", getpid(), getHostName(&clientAddr, buffer));
}
//terminates child and sends Acknoledgement to client when done
void exitClient(int fd, char* buffer){
    sendAcknowledgement(fd, buffer);
    printf("Child %d: Quiting ...\n", getpid());
    free(buffer);
    exit(-1);
}
//make a random portnumber for the data connection
int makePortNumber(int fd, struct sockaddr_in* dataAddr){
    int length = sizeof(struct sockaddr_in);
    int status = getsockname(fd, (struct sockaddr*) dataAddr, (socklen_t*) &length);
    if(status < 0){
        fprintf(stderr, "getsockname faild %s\n", strerror(errno));
        return -1;
    }
    return (ntohs(dataAddr->sin_port)); //used ntohs to make sure the port number is in the right order
}
//send an acknoledgement followed with the port number
void sendPortNumber(int fd, char* buffer, int portNumber){
    buffer[0] = 'A';
    sprintf(buffer + 1, "%d\n", portNumber);
    writeNet(fd, buffer);
}
//change the server directory position
void changeDir(int fd, char* pathname, char* buffer){
    int status = chdir(pathname);
    if(status == 0){
        sendAcknowledgement(fd, buffer);
    }else{
        sendError(fd, buffer);
    }
}
//This function is called when the clients requests to get a file from the server
void sendFile(int client, int fd, char* buffer, char* pathname){
    int filePointer = open(pathname, O_RDONLY, 0644);
    if(filePointer < 0){
        sendError(client, buffer);
        close(fd);
        return;
    }
    sendAcknowledgement(client, buffer);
    writeNetFile(fd, filePointer, buffer);
    close(fd);
    close(filePointer);
}
//the function is called when the clients request to put a file to the server
void getFile(int client, int fd, char* buffer, char* pathname){
    int filePointer = open(pathname, O_WRONLY | O_TRUNC | O_EXCL | O_CREAT, 0644);
    if(filePointer < 0){
        sendError(fd, buffer);
        return;
    }
    sendAcknowledgement(client, buffer);
    readNetFile(fd, filePointer, buffer);
    close(fd);
    close(filePointer);
}

int main(){
    int dataSocketfd, datafd;
    int listenfd = createSocket(); //create a socket to connect to client
    socketBinding(listenfd, &servAdder, MY_PORT_NUMBER); //bind

    if (listen(listenfd, 4) == -1){ //listen up to four connections
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return -1;
    }

    while (true){
        int connectfd = acceptConnection(listenfd, &clientAddr); //accept the connection we listened to
	int n;
        if (fork()){
            close(connectfd);
            waitpid(-1, NULL, WNOHANG);
        }else{
            if (connectfd < 0){
                fprintf(stderr, "Error: %s\n", strerror(errno));
                exit(-1);
            }else{
                char *buffer = malloc(sizeof(char) * 4096);
                char *pathname = malloc(sizeof(char) * 4096);
		        printConnectionInfo(buffer);
                struct timeval tv;
                tv.tv_sec = 60;
                tv.tv_usec = 0;
                setsockopt(connectfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
                
                while (true){
                    n = readNet(connectfd, buffer);
                    if (n <= 0){
                        fprintf(stderr,"Connection to client lost\n");
                        free(buffer);
                        free(pathname);			
                        exit(-1);
                    }	
                    if(strcmp(buffer, "Q\n") == 0){
			            free(pathname);
                        exitClient(connectfd, buffer);
                    }
                    if (strcmp("D\n", buffer) == 0){
                        dataSocketfd = createSocket();
                        socketBinding(dataSocketfd, &servAdder, 0);
                        if (listen(dataSocketfd, 4) == -1){
                            fprintf(stderr, "Error: %s\n", strerror(errno));
                            return -1;
                        }
                        int portNumber = makePortNumber(dataSocketfd, &dataAddr);
                        sendPortNumber(connectfd, buffer, portNumber);
                        datafd = acceptConnection(dataSocketfd, &clientAddr);
                    }
                    if (strcmp("L\n", buffer) == 0){
                        strcpy(buffer, "A\n");
                        writeNet(connectfd, buffer);
                        if (fork()){
                            close(datafd);
                            wait(NULL);
                        }else{
                            dup2(datafd, 1);
			    free(buffer);
			    free(pathname);
                            execlp("ls", "ls", "-l", NULL);
                        }
                    }
                    if(buffer[0] == 'C'){
                        strcpy(pathname, buffer + 1);
                        removeNewLine(pathname);
                        changeDir(connectfd, pathname, buffer);
                    }
                    if(buffer[0] == 'G'){
                        strcpy(pathname, buffer + 1);
                        removeNewLine(pathname);
                        sendFile(connectfd, datafd, buffer, pathname);
                    }
                    if(buffer[0] == 'P'){
                        strcpy(pathname, buffer + 1);
                        removeNewLine(pathname);
                        getFile(connectfd, datafd, buffer, pathname);
                    }
                }
            }
            exit(-1);
        }
    }
    return 1;
}
