

#ifndef A2D_H
#define A2D_H



typedef struct{
    float x;
    float y;
}Points;

void* a2d(void*);

void readVoltage0(void);

void setCurrentVoltage(int curr);

int getCurrentVoltage(void);


void setEndThread(int val);

int getEndThread(void);


void setArraySize(int curr_arr);

int getArraySize(void);


int readVolt(void);

int PWLArraySize(int volt);

float getY(float x);

// void* startThread(void* id);

void stopA2DThread(void);

#endif