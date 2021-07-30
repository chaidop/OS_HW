Polykarpou Christoforos, 2668, chpolykarpou@uth.gr
Tsaggaris Andreas, 2667, antsangaris@uth.gr
Poulianou Chaido, 2613, cpoulianou@uth.gr

-------- IMPLEMENTATION -----------

find_roots_lib: 
	
	After creating the system call.c file in linux-4.5.86/kernel and compiling it, 
	we need to create the wrapper functions and the library.
	In /home , we create roots.c and roots.h and compile them.
	Then we create the static library:
		ar rcs libroots.a roots.o
	Now we can call the function find_roots_wrapper() from any user programm.
	Create the user programm find_roots_lib.c and compile it and link it with the library:
		gcc -c find_roots_lib.c -o find_roots_lib.o
		gcc -o find_roots_lib find_roots_lib.o -L. -lroots

	Run by:
		./find_roots_lib
	Then see the message with:
		dmesg | tail

modules:
	For the modules in the directory project1_modules and the sysfs file in the directory
	sysfs_modules, to compile them run:
		sudo make
	To insert them, run:
		sudo insmod project1-kyber.ko
		sudo insmod sysfs_module.ko
	To remove:
		sudo rmmod project1-kyber.ko
		sudo rmmod sysfs_module.ko

Also, we can use :
	cat /sys/kernel/team13/find_roots
to run the sysfs_module file, which prints the current proccess' pid and all the parents pids with
the command: dmesg | tail.


