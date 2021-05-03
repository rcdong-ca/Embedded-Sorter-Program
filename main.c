
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>

#include "sort.h"
#include "a2d.h"
#include "i2cHandler.h"
#include "networkHandler.h"

int main() {
    pthread_t sort_thread;
    pthread_t network_thread;
    pthread_t a2d_thread;
    pthread_t i2c_thread;
    printf("initiate sort funtions\n");
    pthread_create(&a2d_thread, NULL, a2d, NULL);
    pthread_create(&i2c_thread, NULL, Start_i2c_thread, NULL);
    pthread_create(&sort_thread, NULL, Sorter_startSorting, NULL);
    pthread_create(&network_thread, NULL, StartReceive, NULL);
    pthread_join(a2d_thread, NULL);
    pthread_join(i2c_thread, NULL);
    pthread_join(sort_thread, NULL);
    pthread_join(network_thread, NULL);
    printf("main program ends\n");
    pthread_exit(NULL);
}