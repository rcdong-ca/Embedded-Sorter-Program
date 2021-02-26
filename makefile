
OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror

all: main
	cp udp_test ~/cmpt433/public/myApps/

main: main.c networkHandler.c networkHandler.h sort.c sort.h a2d.c a2d.h i2cHandler.c i2cHandler.h
	$(CC_C) $(CFLAGS) main.c networkHandler.c sort.c a2d.c i2cHandler.c -pthread -lm -o udp_test

clean: 
	rm -rf *o all
