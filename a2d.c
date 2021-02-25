
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "a2d.h"
#include "sort.h"


#define A2D_FILE "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define POINTS_SIZE 10
#define A2D_MAX 4095
#define A2D_VOLTAGE_REF_V 1.8

static Points points[POINTS_SIZE] = {
                                    {0.0, 1.0},
                                    {500.0, 20.0}, 
                                    {1000.0, 60.0},
                                    {1500.0, 120.0},
                                    {2000.0, 250.0},
                                    {2500.0, 300.0},
                                    {3000.0, 500.0},
                                    {3500.0, 800.0},
                                    {4000.0, 1200.0},
                                    {4100.0, 2100.0}
                                };

static int current_voltage  = 0;
static int endThread = 0;
static int curr_array_size = 0;

static pthread_mutex_t array_size_mutex;
static pthread_mutex_t end_thread_mutex;
static pthread_mutex_t voltage_mutex;
static pthread_mutex_t file_mutex;
static pthread_mutex_t read_mutex;

void* a2d(void* x){

    pthread_mutex_init(&array_size_mutex, NULL);
    pthread_mutex_init(&end_thread_mutex, NULL);
    pthread_mutex_init(&voltage_mutex, NULL);
    pthread_mutex_init(&file_mutex, NULL);
    pthread_mutex_init(&read_mutex, NULL);
    
    printf("Starting A2D thread to read Potentiometer\n");
    setEndThread(0);
    // printf("Current End Thread = %d\n", endThread);
    while(endThread == 0){
        // printf("Current End Thread = %d\n", endThread);
        readVoltage0();
        sleep(1);
        // printf("After sleep\n");
        endThread = getEndThread();
    }
    printf("Ending A2D thread\n");
    pthread_exit(NULL);
    return NULL;    

    pthread_mutex_destroy(&array_size_mutex);
    pthread_mutex_destroy(&end_thread_mutex);
    pthread_mutex_destroy(&voltage_mutex);
    pthread_mutex_destroy(&file_mutex);
    pthread_mutex_destroy(&read_mutex);
}

// void* startThread(void* id){
   
// }

void readVoltage0(void){
    pthread_mutex_lock(&read_mutex);
    // printf("Reading Voltage\n");
    int reading = readVolt();
    // double voltage = ((double)reading / A2D_MAX) * A2D_VOLTAGE_REF_V;
    // printf("reading =  %d\n", reading);
    // setCurrentVoltage(reading);
    float array_size = PWLArraySize(reading);
    // printf("voltage =  %f\n", voltage);
    int size_arr = (int) floor((double) array_size);
    Sorter_setArraySize(size_arr);
    printf("New Array Size: %d\n",  size_arr);    
    pthread_mutex_unlock(&read_mutex);
}

int readVolt(void){
    pthread_mutex_lock(&file_mutex);
    // printf("Mutex Locked\n");/
    // Open file
    FILE *f = fopen(A2D_FILE, "r");
    if (!f) {
        printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
        printf(" Check /boot/uEnv.txt for correct options.\n");
        exit(-1);
    }
    // Get reading
    int a2dReading = 0;
    int itemsRead = fscanf(f, "%d", &a2dReading);
    if (itemsRead <= 0) {
        printf("ERROR: Unable to read values from voltage input file.\n");
        exit(-1);
    }
    // Close file
    fclose(f);
    pthread_mutex_unlock(&file_mutex);
    // printf("Mutex unlocked\n");
    return a2dReading;
}


int PWLArraySize(int volt){
    float s = volt;
    float b = ceil( (double) (volt / 500.0)) * 500.0;
    float a = b - 500.0;
    if(b > 4100){
        b = 4100.0;
        a = 4000.0;
    }

    float m = getY(a);
    float n = getY(b);

    float temp1 = (s-a) / (b-a);
    float temp2 = n-m;
    return (temp1)*(temp2)+m;
}

float getY(float x){
    for (int i=0; i < POINTS_SIZE; i++){
        if(x == points[i].x){
            return points[i].y;
        }
    }
    return -1.0;
}

void setArraySize(int curr_arr){
    pthread_mutex_lock(&array_size_mutex);
        curr_array_size = curr_arr;
    pthread_mutex_unlock(&array_size_mutex);

}

int getArraySize(void){    
    pthread_mutex_lock(&array_size_mutex);
        int ret =  curr_array_size;
    pthread_mutex_unlock(&array_size_mutex);

    return ret;
}

void setCurrentVoltage(int curr){
    pthread_mutex_lock(&voltage_mutex);
        current_voltage = curr;
    pthread_mutex_unlock(&voltage_mutex);
}

int getCurrentVoltage(void){
    pthread_mutex_lock(&voltage_mutex);
        int ret =  current_voltage;
    pthread_mutex_unlock(&voltage_mutex);
    return ret;
}

void setEndThread(int val){
    pthread_mutex_lock(&end_thread_mutex);
        endThread = val;
    pthread_mutex_unlock(&end_thread_mutex);
}

int getEndThread(void){
    pthread_mutex_lock(&end_thread_mutex);
        int ret = endThread;
    pthread_mutex_unlock(&end_thread_mutex);
    return ret;
}

void stopA2DThread(void){
    setEndThread(1);
    // printf("End Thread changed to: 5d\n", getEndThread());
}