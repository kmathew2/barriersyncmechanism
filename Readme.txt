##################################################################################################################
#
#					   Filename: README
#					Description: Execution instructions 
#
#	
#	KELVIN THOMAS MATHEW 	1211037370
#	
#
##################################################################################################################

1) Place the patch file into a folder and place the kernel folder along with it.
2) If c:/eosi/Assignment4 is the folder, then the patch will be c:/eosi/Assignment4/patch.diff and the kernel package as c:/eosi/Assignment4/kernel_newbuild
3) Go to the kernel folder from command line as shown below
	cd kernel_newbuild
4) Then run the below code to patch the changes to the kernel_newbuild package
	patch -p2 < ../patch.diff
5) The corresponding file changes to the package will be shown and then we need to build the kernel_newbuild package.
6) Run the below commands to build the kernel,
	export PATH=/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux:$PATH
	ARCH=x86 LOCALVERSION= CROSS_COMPILE=i586-poky-linux- make -j4
	ARCH=x86 LOCALVERSION= INSTALL_MOD_PATH=../galileoinstallation CROSS_COMPILE=i586-poky-linux- make modules_install
	cp arch/x86/boot/bzImage ../galileoinstallation/
7) This would build the package and create the bzImage file in the c:/eosi/Assignment4/galileoinstallation path
8) Copy this bzImage file and paste it in the SD card used for booting the galileo board.
9) Boot the galileo board and it is ready with the corresponding syscalls.
10) Now the test program needs to be transferred to the board. Before that we have set the number of synchronization as a global variable numberofSync to set the required value. The value currently set is 100
11) Place the barrier_usr_pgm.c file with the Makefile into a folder and make the file by running below command
	make clean
	make
12) This would create barrier_usr_pgm.o file which should be transferred to the board using scp as shown below
	sudo scp barrier_usr_pgm.o root@192.168.1.5:/home/
13) Connect to the board using the below command
	sudo screen /dev/ttyUSB0 115200
14) Go to the home directory in the board and run the user program as shown below
	./barrier_usr_pgm.o
15) The program would prompt the average delay between threads in microseconds(like 50), the number of threads for each barrier and their timeout values in nanoseconds(like 1000000000).
16) using dmesg command we can see all the flow of the program execution.


Explaination
------------
The code execution happens as below
1) Barrier init will be invoked and based on the user input the corresponding threads will be created for each barrier for each child.
2) We create two barriers for each child process. There were two child processes created.
3) After the barrier init, the threads will be created and each thread invokes the barrier wait syscall.
4) After the synchronization, barrier destroy syscall will be invoked.
5) The syscall numbers are shown below	
	359 barrier_init
	360	barrier_wait
	361 barrier_destroy


