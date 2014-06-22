
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
        
    //char command[] ="InsertLines 0 2\nHalloText\nAndererText\n";
    //char command[] ="READLINES 0 2\n";
    //debug("Open connection to localhost:3000");
    
/*
    char* commandsToSend[4];
    
    char *command_0 = "InsertLines 0 2\nHalloText\nAndererText\n";
    char *command_1 = "READLINES 0 2";
    char *command_2 = "InsertLines 0 2\nHalloText\nAndererText\n";
    char *command_3 = "InsertLines 2 1\nHalloText2\nAndererText2\n";
    
    commandsToSend[0] = command_0;
    commandsToSend[1] = command_1;
    commandsToSend[2] = command_2;
    commandsToSend[3] = command_3;
*/
    // "INSERTLINES, REPLACELINES, READLINES, DELETELINES, NUMLINES, Quit";
    //create_worker("InsertLines 0 2\nHalloText\nAndererText\n");
/*
    sleep(1);
    create_worker("READLINES 0 2");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText2\nAndererText2\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText2\nAndererText2\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText2\nAndererText2\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText2\nAndererText2\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText2\nAndererText2\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    sleep(1);
    create_worker("READLINES 0 2");
    create_worker("REPLACELINES 0 2\nHalloText3\nAndererText3\n");
*/
    
    create_worker("NUMLINES");
    sleep(1);
    //create_worker("InsertLines 0 2\nHalloText5\nAndererText5\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    create_worker("REPLACELINES 0 2\nHalloText\nAndererText\n");
    //sleep(1);
    //create_worker("READLINES 0 4");
    //sleep(1);
    //create_worker("InsertLines 0 2\nnew insert\nnew insert 2\n");
    //create_worker("DELETELINES 0 5");
    //sleep(3);
    //create_worker("READLINES 0 6");
    sleep(1);
    create_worker("NUMLINES");
    
    //sleep(3);
    
    while (running_threads > 0) {
       sleep(1);
    }
    debug("All workers ended");
    //exit(EXIT_SUCCESS);
    
}