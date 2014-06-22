#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "include/helper.h"
#include "include/process.h"


/*
void sigint(int a) {
    printf("^C caught\n");
    int socket_status, connection_status;
    socket_status = close(socket_server, 2);
    //socket_status = shutdown(socket_server, SHUT_RDWR); // s is socket descriptor, SHUT_RDWR or 2 Further sends and receives are disallowed
    while (socket_status != 0) {
        printf("Wait for shutdown the server\n");
        sleep(1);
    }
    printf("Server is Down\n");
    saveFile();
}
*/


int main(int argc, char *argv[]) {

    
    /*
        int socket_connection;
     */
    pthread_t workers[30];
    int workers_count = 0;

    //signal(SIGINT, sigint);

    struct sockaddr_in socket_adress;
    socket_server = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_server < 0) {
        perror("ERROR, can't create socket\n");
        return -1;
    }
    /*
        bzero((char *) & serv_addr, sizeof (serv_addr));
     */
    socket_adress.sin_family = AF_INET;
    socket_adress.sin_addr.s_addr = INADDR_ANY;
    socket_adress.sin_port = htons(3000);

    if (bind(socket_server, (struct sockaddr *) & socket_adress, sizeof (socket_adress)) < 0) {
        perror("ERROR, can't bind socket to port 3000\n");
        return -1;
    }
    listen(socket_server, 5);

    if (socket_server == -1) {
        return -1;
    }
    debug("Server created and listening. waiting for clients...");

    while (1) {

        int socket_adress_length;
        struct sockaddr_in socket_adress; /* Structure describing an Internet socket address.  */
        socket_adress_length = sizeof (socket_adress);

        /* When a connection arrives, open a new socket to communicate with it */
        socket_connection = accept(socket_server, (struct sockaddr *) & socket_adress, &socket_adress_length);
        if (socket_connection < 0) {
            perror("ERROR, client failed");
            return -1;
        }

        /* Create a new thread, run process->start()
           int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg); */
        pthread_create(&workers[workers_count], NULL, start, (void *) socket_connection);
        workers_count += 1;
        if (workers_count >= 30) {
            workers_count = 0;
        }
    }

    return 0;
}