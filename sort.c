#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "sort.h"

#define MAXLEN 100

//TODO add mutex for all shared variables

static pthread_t sort_thread;
static pthread_mutex_t array_mutex;
static pthread_mutex_t count_mutex;
static pthread_mutex_t arr_len_mutex;
static pthread_mutex_t next_arr_len_mutex;
static pthread_mutex_t sort_flag_mutex;
static long long count = 0;
static int arr_len;
static int next_arr_len;
static bool sort_flag = true;
static int* main_arr = NULL;

void count_inc() {
    pthread_mutex_lock(&count_mutex);
    count++;
    pthread_mutex_unlock(&count_mutex);
}
long long get_count() {
    pthread_mutex_lock(&count_mutex);
    int val = count;
    pthread_mutex_unlock(&count_mutex);
    return val;
}

void printArray(int arr[], int len) {
    for (int i =0; i<len; i++) {
        printf("%d ", arr[i]);
    }
}

void swap(int arr[], int pos1, int pos2) {
    int t = arr[pos1];
    arr[pos1] = arr[pos2];
    arr[pos2] = t;
}

void BubbleSort(int arr[], int len) {
    for (int j=len-1; j>=0; j--) {
        for (int i =0; i<j; i++) {
            if (arr[i] > arr[i+1]) {
                pthread_mutex_lock(&array_mutex);
                swap(arr, i, i+1);
                pthread_mutex_unlock(&array_mutex);
            }
            //receive notice to stop the sort!
            pthread_mutex_lock(&sort_flag_mutex);
            if (sort_flag==false) {
                pthread_mutex_unlock(&sort_flag_mutex);
                 return;
            }
            pthread_mutex_unlock(&sort_flag_mutex);
        }
    }
}

void permutation(int* arr, int len) {
    //requires only one so just shuffle the array
    for (int i =0; i< len-1; i++) {
        int n = i + rand() / (RAND_MAX / (len-1-i)+1 ); //FISH-yates shuffle
        swap(arr, i, n);
    }
}

void* sort_thread_task() {
    arr_len = next_arr_len;
    main_arr = (int*) malloc(sizeof(int) * MAXLEN);
    pthread_mutex_lock(&array_mutex);   //initialize the arrays
    for (int i =0; i< MAXLEN; i++) {
        main_arr[i] = i;
    }
    pthread_mutex_unlock(&array_mutex);

    while (1) {
        pthread_mutex_lock(&array_mutex);
        permutation(main_arr, arr_len);
        pthread_mutex_unlock(&array_mutex);
        BubbleSort(main_arr, arr_len);
        if (sort_flag) {
            count_inc();
        }
        else 
            break;
    }
    printf("Exiting thread\n");
    pthread_exit(NULL);
}
void Sorter_startSorting(void) {
    void * status = NULL;
    next_arr_len = MAXLEN; //TO CHANGE
    pthread_mutex_init(&array_mutex, NULL);
    pthread_mutex_init(&arr_len_mutex, NULL);
    pthread_mutex_init(&next_arr_len_mutex, NULL);
    pthread_mutex_init(&sort_flag_mutex, NULL);
    pthread_mutex_init(&count_mutex, NULL);
    pthread_create(&sort_thread, NULL, sort_thread_task, NULL);
    //Testing purposes
    // sleep(3);
    // int len;
    // int * res = Sorter_getArrayData(&len);
    // long long mycount = Sorter_getNumberArraysSorted();
    // printf("count = %lld len = %d\n", mycount, len);
    // Sorter_stopSorting();
    int rc = pthread_join(sort_thread, NULL);
    if (rc) {
        printf("ERROR; return code from pthread_join() is %d\n", rc);
        exit(-1);
    }
    printf("Main: completed join with thread having a status\n");
    pthread_mutex_destroy(&array_mutex);
    pthread_mutex_destroy(&next_arr_len_mutex);
    pthread_mutex_destroy(&next_arr_len_mutex);
    pthread_mutex_destroy(&sort_flag_mutex);
    pthread_mutex_destroy(&count_mutex);
}

//this should be executed by main thread
void Sorter_stopSorting(void) {
    pthread_mutex_lock(&sort_flag_mutex);
    sort_flag = false;
    pthread_mutex_unlock(&sort_flag_mutex);
}

void Sorter_setArraySize(int newSize) {
    pthread_mutex_lock(&next_arr_len_mutex);
    next_arr_len = newSize;
    pthread_mutex_unlock(&next_arr_len_mutex);
}
int Sorter_getArrayLength(void) {
    pthread_mutex_lock(&arr_len_mutex);
    return arr_len;
    pthread_mutex_unlock(&arr_len_mutex);
}

int* Sorter_getArrayData(int *length) {
    *length = arr_len;
    int* dup_arr = (int*)malloc(sizeof(int) * arr_len);
    pthread_mutex_lock(&array_mutex);
    for (int i =0; i<arr_len; i++) {
        dup_arr[i] = main_arr[i];
    }
    pthread_mutex_unlock(&array_mutex);
    return dup_arr;
}

int Sorter_getArray_Value(int n) {
    int val =0;
    pthread_mutex_lock(&next_arr_len);
    if (n > next_arr_len) {
        val  = next_arr_len * -1;       // -negative will tell us invalid command!
    }
    pthread_mutex_unlock(&array_mutex);
    if (val>-1) {
        pthread_mutex_lock(&array_mutex);
        val = main_arr[n];
        pthread_mutex_unlock(&array_mutex);
    }
    return val;
}

long long Sorter_getNumberArraysSorted(void) {
    return get_count();
}
int main() {
    Sorter_startSorting();
    return 0;
}