
#变量定义—内核的build路径
KERNDIR:=/lib/modules/5.0.0-29-generic/build
#KERNDIR:=/opt/qt5pi/sysroot_2/usr/src/linux-headers-4.19.57-v7+
#变量定义通过shell 的pwd命令获取
PWD:=$(shell pwd)
#ARCH:=arm
#CROSS_COMPILE:=/opt/qt5pi/gcc-linaro-5.5.0-2017.10-i686_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
#CC=$(CROSS_COMPILE)gcc
#LD:= $(CROSS_COMPILE)ld
#连接文件
obj-m:=device.o
all:
	#编译命令 模块
	make -C $(KERNDIR) M=$(PWD) modules
clean:
	#清除模块命令
	make -C $(KERNDIR) M=$(PWD) clean 
