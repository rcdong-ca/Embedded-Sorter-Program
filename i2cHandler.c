// Assume pins already configured for I2C: 
//   (bbg)$ config-pin P9_18 i2c
//   (bbg)$ config-pin P9_17 i2c

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <pthread.h>
#include "sort.h"
#define I2C_DEVICE_ADDRESS 0x20
#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

#define GPIO_61 "/sys/class/gpio/gpio61/value"  //Left Display
#define GPIO_44 "/sys/class/gpio/gpio44/value"  //Right display

static int bot_num[] = {0xA1, 0x80, 0x31, 0xB0, 0x90, 0xB0, 0xB1,
                         0x80, 0xB1, 0xB0};
static int top_num[] = {0x86, 0x02, 0xE, 0xE, 0x8A, 0x8C, 0x8C,
                          0x06, 0x8E, 0x8E};
static int count = 0;
static int exit_flag = 1;
static pthread_mutex_t count_mutex;
static pthread_mutex_t exit_flag_mutex;
static pthread_t display_thread;


void set_count(int n) {
    pthread_mutex_lock(&count_mutex);
    count = n; //counts sorted per second
    pthread_mutex_unlock(&count_mutex);
}

int get_count() {
    pthread_mutex_lock(&count_mutex);
    int val = count;
    pthread_mutex_unlock(&count_mutex);
    return val;
}

void set_exit_flag() {
    pthread_mutex_lock(&exit_flag_mutex);
    exit_flag = 0;
    pthread_mutex_unlock(&exit_flag_mutex);
}
int get_exit_flag() {
    pthread_mutex_lock(&exit_flag_mutex);
    int val = exit_flag_mutex;
    pthread_mutex_unlock(&exit_flag_mutex);
    return val;
}

//Turn off, on GPIO pins!
void set_GPIO_pin(int pin_num, int val) {
    FILE* fd;
    if (pin_num == 61)
        fd = fopen(GPIO_61, "w");
    else if  (pin_num==44)
        fd = fopen(GPIO_44, "w");
    else {
        printf("Incorrect GPIO num: %d\n", pin_num);
        return;
    }
    fprintf(fd,"%d", val);
    fclose(fd);
    return ;
}

static int initI2cBus(char* bus, int address){
    int i2cFileDesc = open(bus, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    return i2cFileDesc;
}


static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value){
    unsigned char buff[2];
    buff[0] = regAddr;
    buff[1] = value;
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2) {
        perror("I2C: Unable to write i2c register."); 
        exit(1);
        }
    }

static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr){
    // To read a register, must first write the address
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr)) {
        perror("I2C: Unable to write to i2c register.");
        exit(1);
    }
    // Now read the value and return it
    char value = 0;
    res = read(i2cFileDesc, &value, sizeof(value));
    if (res != sizeof(value)) {
        perror("I2C: Unable to read from i2c register");
        exit(1);
    }
    return value;
}

void display_number(int i2c_fd) {
    long long curr_count = get_count();
    int prev_count = 0;
    int left_n = 0;
    int right_n = 0;
    printf("leftn = %d, rightn = %d\n", left_n, right_n);
    clock_t start, end;
    while(get_exit_flag==1) {
        start = clock();
        end = clock();
        while ( (double)(end-start) <= 1.0) {             //TODO: flag for exiting thread
            if (curr_count > 99) 
                curr_count = 99;
            left_n = (int)curr_count / 10;
            right_n = (int)curr_count%10;
            set_GPIO_pin(44, 0);
            set_GPIO_pin(61, 0);
            writeI2cReg(i2c_fd,REG_OUTA, bot_num[left_n] );
            writeI2cReg(i2c_fd, REG_OUTB, top_num[left_n]);
            set_GPIO_pin(61, 1);
            sleep(0.005);
            
            set_GPIO_pin(61, 0);
            writeI2cReg(i2c_fd,REG_OUTA, bot_num[right_n]);
            writeI2cReg(i2c_fd, REG_OUTB, top_num[right_n]);
            set_GPIO_pin(44,1);
            sleep(0.005);
            end = clock();
        }
        prev_count = curr_count;
        curr_count = get_count() - prev_count;
    }
    pthread_exit(NULL);
}

int main(){
    printf("Drive display (assumes GPIO #61 and #44 are output and 1\n");
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00); //  
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00); //sets devuce to be output on all pins


    pthread_mutex_init(&count_mutex, NULL);
    pthread_mutex_init(&set_exit_flag, NULL);
    pthread_create(display_thread, NULL, display_number, i2cFileDesc);

    clock_t start, end;
    start = clock();
    end = clock();
    while (get_exit_flag==1) {
        set_count(0);
        curr_count = Sorter_getNumberArraysSorted();
        set_count(curr_count);
        end = clock();
        if ( (double)(end - start) <6)) {
            set_exit_flag();
        }
    }
    printf("Exiting i2c handler program\n");
    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&set_exit_flag);
    // Read a register:
    unsigned char regVal = readI2cReg(i2cFileDesc, REG_OUTA);
    printf("Reg OUT-A = 0x%02x\n", regVal);
    close(i2cFileDesc);
    return 0;
}
