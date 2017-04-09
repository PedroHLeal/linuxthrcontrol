obj-m+=throttlectl.o
 
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
load:
	rmmod throttlectl
	insmod throttlectl.ko
write:
	echo "thon" > /proc/throttlectl
read:
	cat /proc/throttlectl
writeS:
	echo "thon" > /proc/throttlectl
	cat /proc/throttlectl
run:
	rmmod throttlectl
	insmod throttlectl.ko
	echo "thon" > /proc/throttlectl
	cat /proc/throttlectl
check:
	lsmod | grep throttlectl