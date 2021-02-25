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

#define PORT 12345
#define MAX_BUFF_SIZE 65000
#define MAX_PACK_SIZE 1024

int help_command(char* snd_buffer) {
    printf("TEST1\n");
    sprintf(snd_buffer, "Accepted command examples:\
            count       -- display number arrays sorted.\n\
            get length  -- display lenght of array currently being sorted.\n\
            get array   -- display the full array being sorted.\n\
            get 10      -- display the tenth element of array currently being sorted.\n\
            stop        -- cause the server program to end.\n");
    printf("TEST2\n");
    return strlen(snd_buffer);
}

int count_command(char* snd_buff) {
    return sprintf(snd_buff, "%lld", Sorter_getNumberArraysSorted() ); //return the length of the long long
}

int get_n_command(char* snd_buff, int n) {
    int val = Sorter_getArray_Value(n);
    if (val < 0) {
        return snprintf(snd_buff, MAX_BUFF_SIZE,
                "Invalid Argument. Must be between 1 and %d (array length).", val*-1);
    }
    return snprintf(snd_buff, MAX_BUFF_SIZE,"Value %d = %d", n, val);
}

int get_length_command(char* snd_buff) {
    return sprintf(snd_buff, "%d", Sorter_getArrayLength() ) ;
}
int get_array_command(char* snd_buff) {
    int res_len =0;
    int* res_arr = Sorter_getArrayData(&res_len);
    printf("res_len =%d \n", res_len);
    int index = 0;
    int i=0;
    for (i =0; i<res_len-1; i++) {          //REFACTOR: INCOMING IS GREATER THAN BUFF SIZE
        index+=sprintf(snd_buff + index, "%d,", res_arr[i]);
    }
    index+=sprintf(snd_buff + index, "%d", res_arr[i]);
    printf("res_len = %d   index = %d\n", res_len, index);
    // if (res_len!=index) {
    //     perror("GET_ARRAY_COMMAND erorr: saved arraylen != array len");
    //     exit(1);
    // }
    // free( (int**)res_arr );
    // res_arr = NULL;
    return index; 
}

int stop_command(char* snd_buff) {
    Sorter_stopSorting();
    stopA2DThread();
    return -1 * snprintf(snd_buff, MAX_BUFF_SIZE, "Program Terminating");
}

int handle_packet(char recv_buffer[], char snd_buffer[], int msg_len) {
    //case 1: help command
    int snd_len =0;
    if (strncmp(recv_buffer, "help\n", msg_len) ==0 ) {
        printf("1\n");
        snd_len= help_command(snd_buffer);
        
    }
    else if (strncmp(recv_buffer, "count\n", msg_len)==0)
    {
        //printf("2\n");
        snd_len= count_command(snd_buffer);
    }
    else if (strncmp(recv_buffer, "get length\n", msg_len)==0) {
        //printf("3\n");
        snd_len =  get_length_command(snd_buffer);
    }
    else if (strncmp(recv_buffer, "get array\n", msg_len)==0) {
        printf("4\n");
        snd_len= get_array_command(snd_buffer); 
    }
    else if (strncmp(recv_buffer, "stop\n", msg_len) ==0) {
        //printf("5\n");
        snd_len= stop_command(snd_buffer);
    }
    else {
        //check if the string is longer than 5 characters
        if (msg_len < 5 && strncmp(recv_buffer+3, " ", 1)!=0 && ( 
            strncmp(recv_buffer+msg_len-1, "\n", 1)!=0 || isdigit(recv_buffer[msg_len-1])<1)) {
            //printf("Invalid command: %s\n", recv_buffer);
            return snprintf(snd_buffer, MAX_BUFF_SIZE, "Invalid Command, please type help\n");
            
        }
        //almost valid, lets check that after it is all digits
        for (int i =4 ; i<msg_len-1; i++) {
            if (isdigit(recv_buffer[i])<1)  {
                return snprintf(snd_buffer, MAX_BUFF_SIZE, "Invalid Command, please type help\n");
            }
        }
        // recv_buffer[msg_len] = "\0";
        //printf("%s is valid command\n", recv_buffer);
        //it is a valid command:
        int val = (int)strtol( recv_buffer+4, (char **)NULL, 10);
        snd_len = get_n_command(snd_buffer, val);
    }
    return snd_len;
}

void* StartReceive(void* t) {
    char recv_buffer[MAX_BUFF_SIZE];
    char send_buffer[MAX_BUFF_SIZE];
    struct sockaddr_in target_addr;

    //create socket fd
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd==0) {
        perror("Socket fail to create");
        exit(-1);
    }
    memset(&target_addr, 0, sizeof(target_addr));
    //memset(&target_addr, 0, sizeof(target_addr));

    target_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    target_addr.sin_port = htons(PORT);
    target_addr.sin_family = AF_INET;

    bind(sock_fd, (struct sockaddr*)&target_addr, sizeof(target_addr));

    socklen_t target_struct_len;
    int stop_flag = 0;
    char* send_msg = (char*) malloc(sizeof(char)*MAX_PACK_SIZE);
    while(stop_flag >-1) { //have a flag set here to cancel this thread when done
        target_struct_len = sizeof(target_addr);
        printf("waiting for packets on port %d....\n", PORT);
        int msg_len = recvfrom(sock_fd, (char *)recv_buffer, MAX_BUFF_SIZE, 0, 
                (struct sockaddr*)&target_addr, &target_struct_len);
        //printf("msg received: %s:\n ", recv_buffer);
        printf("msg_len = %d\n", msg_len);
        int send_len = handle_packet(recv_buffer,send_buffer, msg_len);
        printf("test3\n");
        if (send_len <0) {
            stop_flag = send_len;
            send_len*=-1;
        }
        //design choice: first byte 1 or 0. 1: more packet. 0: last packet
        int end = MAX_PACK_SIZE-1;
        int start = 0;

        printf("send buyffer = %s\n", send_buffer);
        while (send_len > MAX_PACK_SIZE) { //only fthe get array command
            while (send_buffer[end]!=',' && end!=start) {       //end of number
                end--;
            }
            end+=1;
            strncpy(send_msg, send_buffer + start, end - start);
            printf("send mssage_in = %s\n", send_msg);
            sendto(sock_fd, send_msg, end - start, 0, (struct sockaddr *)&target_addr, target_struct_len);
            send_len = send_len - (end -start);
            start = end;
            end = start + MAX_PACK_SIZE-1;
            printf("Next packet\n!");
        }
        printf("Sending the last packet\n");
        strncpy(send_msg, send_buffer + start, start + send_len);
        printf("send msg %s\n",send_msg);
        sendto(sock_fd, send_msg, end-start, 0, (struct sockaddr *)&target_addr, target_struct_len);

        memset(recv_buffer, 0, MAX_BUFF_SIZE);
         memset(send_buffer, 0, MAX_BUFF_SIZE);
          memset(send_msg, 0, MAX_PACK_SIZE);
    }
    printf("networkprogram ends\n");
    pthread_exit(NULL);
}

int main() {
    pthread_t sort_thread;
    pthread_t network_thread;
    pthread_t a2d_thread;
    printf("initiate sorth funtions\n");
    pthread_create(&a2d_thread, NULL, a2d, NULL);
    pthread_create(&sort_thread, NULL, Sorter_startSorting, NULL);
    pthread_create(&network_thread, NULL, StartReceive, NULL);
    pthread_join(a2d_thread, NULL);
    pthread_join(sort_thread, NULL);
    pthread_join(network_thread, NULL);
    printf("main program ends\n");
    return 0;
}