#ifndef _I2C_HANDLER_H_
#define _I2C_HANDLER_H_


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

void* Start_i2c_thread(void*);
void* display_number(void* fd);
void stop_i2c(void);

#endif