
#include<stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include "include/helper.h"


volatile int running_threads = 0; // Volatile tells the compiler not to optimize anything that has to do with the volatile variable.
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;


int send_command(char *command) {
    
//int send_command(char *command) {
    //sleep(10);
    int connection;
    connection = con_open(3000);
    if (!connection) {
        perror("ERROR, Verbindungsfehler\n");
        return -1;
    }
    debug("Open Stream");

/*
    stream = fdopen(connection, "w");
    if (stream == NULL) {
        perror("ERROR, Write Stream Fehler\n");
        return -1;
    }
*/
    
    debug("Write data to stream");
    debug(command);
    // extern size_t fwrite (const void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __s);
    //if (fwrite(command, 1, sizeof(command), stream) <= 0) {
    if( send(connection , command , strlen(command) , 0) < 0){
        perror("ERROR, Ungültiges Packet.");
        return -1;
    }
    
    char server_reply[512];
    //Receive a reply from the server
    int b = recv(connection, server_reply , 6000 , 0);
    if(b < 0)
    {
        puts("recv failed");
    }
    puts("Reply received\n");
    puts(server_reply);
    
/*
    char buffer[512];
    ssize_t nBytes;
    nBytes = recv(stream, buffer, sizeof (buffer) - 1, 0);
    if (nBytes <= 0) {
        perror("ERROR, Ungültiges Packet.");
        return -1;
    }
    printf("Response: %s\n", buffer);
*/
    debug("Finished, closing..");
    //fflush(stream);
    //fclose(stream);
    close(connection);
    
    pthread_mutex_lock(&running_mutex);
    running_threads--;
    pthread_mutex_unlock(&running_mutex);
    
    return 0;
}


int con_open(int port) {
    int sockfd;
    struct sockaddr_in server;
    struct hostent *host_addr = gethostbyname("localhost");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR, Socket Fehler\n");
        return -1;
    }
    if (!host_addr) {
        perror("ERROR, DNS fail\n");
        return -1;
    }
    memcpy(&server.sin_addr, host_addr->h_addr_list[0], host_addr->h_length);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server))) {
        perror("ERROR, Socket Fehler\n");
        return -1;
    }
    return sockfd;
}

pthread_t workers[50];
int commandNr = 0;

int create_worker(char* command){
    debug("create worker");
    //char* command = commandsToSend[commandNr];
    
    pthread_mutex_lock(&running_mutex);
    running_threads++;
    pthread_mutex_unlock(&running_mutex);
    
    pthread_create(&workers[commandNr], NULL, send_command, (void *) command);
    commandNr++;
    return 1;
}

main(int argc, char* argv[]){
    debug("Test client");
        
    create_worker("NUMLINES");
    sleep(3);
    create_worker("InsertLines 0 2\nZeile 6\nZeile6\n");
    create_worker("InsertLines 0 2\nZeile 3\nZeile4\n");
    create_worker("InsertLines 0 2\nZeile 1\nZeile2\n");
    sleep(3);
    create_worker("READLINES 0 6");
    sleep(3);
    create_worker("REPLACELINES 0 2\nZeile 1 neu\nZeile 2 neu\n");
    sleep(3);
    create_worker("READLINES 0 6");
    create_worker("NUMLINES");
    sleep(3);
    create_worker("DELETELINES 0 5");
    sleep(3);
    create_worker("NUMLINES");
    create_worker("READLINES 0 6");
    sleep(3);
    create_worker("READLINES 0 1");
    
    while (running_threads > 0) {
       sleep(1);
    }
    debug("All workers ended");
    //exit(EXIT_SUCCESS);
    
}