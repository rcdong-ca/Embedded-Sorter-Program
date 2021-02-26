
OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror

# all: main
# 	cp udp_test ~/cmpt433/public/myApps/

# main: networkHandler.c networkHandler.h sort.c sort.h a2d.c a2d.h
# 	$(CC_C) $(CFLAGS) networkHandler.c sort.c a2d.c -pthread -lm -o udp_test

all: main
	cp i2c_test ~/cmpt433/public/myApps/
main: i2cHandler.c sort.c sort.h
	$(CC_C) $(CFLAGS) i2cHandler.c sort.c -pthread -o i2c_test

clean: 
	rm -rf *o all


# all:
# 	arm-linux-gnueabihf-gcc -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror hello.c -o hello
# 	cp hello ~/cmpt433/public/myApps/

