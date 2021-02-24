

#ifndef A2D_H
#define A2D_H


#define A2D_FILE "~/sys/bus/iio/devices/iio:device0/in_voltage0_raw"



static struct Points{
    float x;
    float y;
};

#define POINTS_SIZE 10
static struct Points points[POINTS_SIZE] = {
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

static int A2D_MAX = 4095;
static int current_voltage  = 0;
static int endThread = 0;
static int curr_array_size = 0;

static pthread_mutex_t array_size_mutex;
static pthread_mutex_t end_thread_mutex;
static pthread_mutex_t voltage_mutex;
static pthread_mutex_t file_mutex;

void init(void);

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

#endif