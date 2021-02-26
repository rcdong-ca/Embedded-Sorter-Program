#ifndef _NetHandler_H_
#define _NetHandler_H_

#define PORT 12345
#define MAX_BUFF_SIZE 1024
#define MAX_PACK_SIZE 1024


void* StartReceive(void* t);
void stop_I2c();

#endif