#ifndef _NetHandler_H_
#define _NetHandler_H_

#define PORT 12345
#define MAX_BUFF_SIZE 65000
#define MAX_PACK_SIZE 1024

void* StartReceive(void* t);
int handle_packet(char recv_buffer[], char snd_buffer[], int msg_len);
int help_command(char* snd_buffer);
int count_command(char* snd_buff);
int get_n_command(char* snd_buff, int n);
int get_length_command(char* snd_buff);
int get_array_command(char* snd_buff);
int stop_command(char* snd_buff);

#endif