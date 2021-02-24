
#include "a2d.h"
#include "sort.h"



void a2d(void){
    pthread_t thread_id;

    pthread_mutex_init(&array_size_mutex, NULL);
    pthread_mutex_init(&end_thread_mutex, NULL);
    pthread_mutex_init(&voltage_mutex, NULL);
    pthread_mutex_init(&file_mutex, NULL);
    
    pthread_create(&thread_id, NULL, startThread, (void*)&thread_id);     

    pthread_mutex_destroy(&array_size_mutex);
    pthread_mutex_destroy(&end_thread_mutex);
    pthread_mutex_destroy(&voltage_mutex);
    pthread_mutex_destroy(&file_mutex);


    //May need to make ptr = nulll and destroy ptr
}

void* startThread(void* id){
    printf("Starting A2D thread to read Potentiometer\n");
    setEndThread(0);
    while(endThread == 0){
        readVoltage0();
    }
    printf("Ending A2D thread\n");
    pthread_exit(NULL);
    return NULL;
}

void readVoltage0(void){

    int reading = readVolt();
    // setCurrentVoltage(reading);
    float array_size = PWLArraySize(reading);
    int size_arr = (int) floor(size_arr);
    Sorter_setArraySize(size_arr);
    printf("New Array Size: %d",  size_arr);
    sleep(1);
    
}

int PWLArraySize(int volt){
    float s = volt;
    float b = ceil(volt / 500.0) * 500.0;
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
    {
        curr_array_size = curr_arr;
    }
    pthread_mutex_unlock(&array_size_mutex);

}

int getArraySize(void){    
    return curr_array_size;
}

void setCurrentVoltage(int curr){
    pthread_mutex_lock(&voltage_mutex);
    {
        current_voltage = curr;
    }
    pthread_mutex_unlock(&voltage_mutex);
}

int getCurrentVoltage(void){
    return current_voltage;
}

void setEndThread(int val){
    pthread_mutex_lock(&end_thread_mutex);
    {
        endThread = val;
    }
    pthread_mutex_unlock(&end_thread_mutex);
}

int getEndThread(void){
    return endThread;
}

int readVolt(void){
    pthread_mutex_lock(&file_mutex);
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
    return a2dReading;
    pthread_mutex_unlock(&file_mutex);
}
int main(){
    a2d();
    return 0;
}