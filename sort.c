#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "sort.h"
#include "a2d.h"

#define MAXLEN 2048

//TODO add mutex for all shared variables

static pthread_mutex_t array_mutex;
static pthread_mutex_t count_mutex;
static pthread_mutex_t next_arr_len_mutex;
static pthread_mutex_t sort_flag_mutex;
static pthread_mutex_t arr_len_mutex;
static int arr_len = 0;
static long long count = 0;
static int next_arr_len;
static bool sort_flag = true;
static int* main_arr = NULL;

void increase_count() {
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

void set_sort_flag(bool val) {
    pthread_mutex_lock(&sort_flag_mutex);
    sort_flag = val;
    pthread_mutex_unlock(&sort_flag_mutex);
}
bool get_sort_flag() {
    pthread_mutex_lock(&sort_flag_mutex);
    bool val = sort_flag;
    pthread_mutex_unlock(&sort_flag_mutex);
    return val;
}

void set_next_array_len(int val) {
    pthread_mutex_lock(&next_arr_len_mutex);
    next_arr_len = val;
    pthread_mutex_unlock(&next_arr_len_mutex);
}
int get_next_array_len() {
    pthread_mutex_lock(&next_arr_len_mutex);
    int val = next_arr_len;
    pthread_mutex_unlock(&next_arr_len_mutex);
    return val;
}

bool compare_arr_len(int cur_len) {
    bool val;
    pthread_mutex_lock(&next_arr_len_mutex);
    if (cur_len == next_arr_len) 
        val = true;
    else
        val = false;
    pthread_mutex_unlock(&next_arr_len_mutex);
    return val;
}

void set_arr_len() {
    pthread_mutex_lock(&arr_len_mutex);
    arr_len = get_next_array_len();
    pthread_mutex_unlock(&arr_len_mutex);
}
int get_arr_len() {
    pthread_mutex_lock(&arr_len_mutex);
    int val = arr_len;
    pthread_mutex_unlock(&arr_len_mutex);
    return val;
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
            {
                if (get_sort_flag()==false && compare_arr_len(len)==false ) {
                    break;
                }
            }
        }
    }
}

void permutation(int* arr, int len) {
    pthread_mutex_lock(&array_mutex);
    //requires only one so just shuffle the array
    for (int i =0; i< len-1; i++) {
        int n = i + rand() / (RAND_MAX / (len-1-i)+1 ); //FISH-yates shuffle
        swap(arr, i, n);
    }
    pthread_mutex_unlock(&array_mutex);
}

void sort_thread_task() {
    int c = 0;
    while (get_sort_flag() ) {
        set_arr_len();
        int cur_arr_len = get_arr_len();
        main_arr = (int*) malloc(sizeof(int) * cur_arr_len);
        pthread_mutex_lock(&array_mutex);   //initialize the arrays
        for (int i =0; i< cur_arr_len; i++) {
            main_arr[i] = i;
        }
        pthread_mutex_unlock(&array_mutex);
        printf("c = %d\n", c);
        //Sorting the array with current length
        while ( get_sort_flag() && compare_arr_len(cur_arr_len)) {
            permutation(main_arr, cur_arr_len);
            BubbleSort(main_arr, cur_arr_len);
            increase_count();
        } 
        free(main_arr);
        main_arr = NULL; 
    }
}
void* Sorter_startSorting(void* t) {
    next_arr_len = Sorter_getArrayLength(); //TO CHANGE
    pthread_mutex_init(&array_mutex, NULL);
    pthread_mutex_init(&next_arr_len_mutex, NULL);
    pthread_mutex_init(&sort_flag_mutex, NULL);
    pthread_mutex_init(&count_mutex, NULL);
    sort_thread_task();
    printf("Sorting Alg closing\n");
    //pthread_create(&sort_thread, NULL, sort_thread_task, NULL);
    //Testing purposes
    // sleep(3);
    // int len;
    // int * res = Sorter_getArrayData(&len);
    // long long mycount = Sorter_getNumberArraysSorted();
    // printf("count = %lld len = %d\n", mycount, len);
    // Sorter_stopSorting();
    // int rc = pthread_join(sort_thread, NULL);
    // if (rc) {
    //     printf("ERROR; return code from pthread_join() is %d\n", rc);
    //     exit(-1);
    // }
    // printf("Main: completed join with thread having a status\n");
    pthread_mutex_destroy(&array_mutex);
    pthread_mutex_destroy(&next_arr_len_mutex);
    pthread_mutex_destroy(&next_arr_len_mutex);
    pthread_mutex_destroy(&sort_flag_mutex);
    pthread_mutex_destroy(&count_mutex);
    pthread_exit(NULL);
}

//this should be executed by main thread
void Sorter_stopSorting(void) {
    set_sort_flag(false);
}

void Sorter_setArraySize(int new_size) {
    set_next_array_len(new_size);
}
int Sorter_getArrayLength(void) {
    return get_arr_len();
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
    {
        pthread_mutex_lock(&next_arr_len_mutex);
        if (n > next_arr_len) {
            val  = next_arr_len * -1;       // -negative will tell us invalid command!
        }
        pthread_mutex_unlock(&next_arr_len_mutex);
    }
    if (val>-1) {
        {
            pthread_mutex_lock(&array_mutex);
            val = main_arr[n];
            pthread_mutex_unlock(&array_mutex);
        }
    }
    return val;
}

long long Sorter_getNumberArraysSorted(void) {
    return get_count();
}