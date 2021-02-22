#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>

#include "sort.h"
#define PORT 12345
#define MAX_BUFF_SIZE 1024
#define MAX_PACK_SIZE 1024

//STATIC FOR MUTEX FOR EACH BUFFER ACCESS
//STATIC FOR SEMAPHORE FOR FULLNESS OF BUFFER

void StartReceive(void) {
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

    target_addr.sin_addr.s_addr = INADDR_ANY;
    target_addr.sin_port = htons(PORT);
    target_addr.sin_family = AF_INET;

    while(1) { //have a flag set here to cancel this thread when done
        int target_struct_len;
        int msg_len = recvfrom(sock_fd, (char *)recv_buffer, MAX_BUFF_SIZE, 0, 
                (struct sockaddr*)&target_addr, &target_struct_len);
        recv_buffer[msg_len] = '\0';

        int send_len = handle_packet(recv_buffer,send_buffer, msg_len);

        //design choice: first byte 1 or 0. 1: more packet. 0: last packet
        int end = MAX_PACK_SIZE-1;
        int start = 0;
        char* send_msg = malloc(sizeof(char)*MAX_PACK_SIZE);
        while (send_len > MAX_PACK_SIZE) { //only fthe get array command
            send_msg[0] = "1";
            while (send_buffer[end]!=",") {       //end of number
                end--;
            }
            strncpy(send_msg+1, send_buffer + start, end);
            sendto(sock_fd, send_msg, end+1 - start, 0, &target_addr, &target_struct_len);
            send_len-=end;
        }
        send_msg[0]= "0";
        strncpy(send_msg+1, send_buffer + start, send_len);
        sendto(sock_fd, send_msg, send_len+1 - start, 0, &target_addr, &target_struct_len);
    }
}

int handle_packet(char recv_buffer[], char snd_buffer[], int msg_len) {
    //case 1: help command
    if (strncmp(recv_buffer, "help", msg_len) ==0 ) {
        return help_command(snd_buffer);
        
    }
    else if (strncmp(recv_buffer, "count", msg_len)==0)
    {
        printf("2\n");
        return count_command(snd_buffer);
    }
    else if (strncmp(recv_buffer, "get length", msg_len)==0) {
        printf("3\n");
        return get_length_command(snd_buffer);
    }
    else if (strncmp(recv_buffer, "get array", msg_len==0)) {
        printf("4\n");
        return get_array_command(snd_buffer); 
    }
    else if (strncmp(recv_buffer, "stop", msg_len) ==0) {
        printf("5\n");
        return stop_command(snd_buffer);
    }
    else {
        //check if the string is longer than 5 characters
        if (msg_len < 5 && strncmp(recv_buffer+3, " ", 1)!=0 && ( 
            strncmp(recv_buffer+msg_len-1, "\n", 1)!=0 || isdigit(recv_buffer[msg_len-1])<1)) {
            printf("Invalid command: %s\n", recv_buffer);
            return snprintf(snd_buffer, MAX_BUFF_SIZE, "Invalid Command, please type help\n");
            
        }
        //almost valid, lets check that after it is all digits
        for (int i =4 ; i<msg_len-1; i++) {
            if (isdigit(recv_buffer[i])<1)  {
                return snprintf(snd_buffer, MAX_BUFF_SIZE, "Invalid Command, please type help\n");
            }
        }
        recv_buffer[msg_len] = "\0";
        printf("%s is valid command\n", recv_buffer);
        //it is a valid command:
        int val = (int)strtol(recv_buffer[4], (char **)NULL, 10);
        val = get_n_command(snd_buffer, val);
    }
}

int help_command(char* snd_buffer) {
    sprintf(snd_buffer, "Accepted command examples:\
            count       -- display number arrays sorted.\n\
            get length  -- display lenght of array currently being sorted.\n\
            get array   -- display the full array being sorted.\n\
            get 10      -- display the tenth element of array currently being sorted.\n\
            stop        -- cause the server program to end.\n");
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
    int index = 0;
    for (int i =0; i<res_len-1; i++) {
        index+=snprintf(snd_buff + index, MAX_BUFF_SIZE - index, "%d,", res_arr[i]);
    }
    index+=snprintf(snd_buff + index, MAX_BUFF_SIZE - index, "%d", res_arr[res_len-1]);
    if (res_len!=index) {
        perror("GET_ARRAY_COMMAND erorr: saved arraylen != array len");
        exit(1);
    }
    return res_len; 
}

int stop_command(char* snd_buff) {
    Sorter_stopSorting();
    return snprintf(snd_buff, MAX_BUFF_SIZE, "Program Terminating");
}
//TODO: WE GONNA TEST THIS OUT