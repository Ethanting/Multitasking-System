#-------------------------- Makefile --------------------------

VOS: main.c proc_func.c queue.c ts.s mount_root.c ext2_func.c util.c
	gcc -g -m32 main.c proc_func.c queue.c ts.s mount_root.c ext2_func.c util.c -o VOS
disk:
	rm mydisk
	dd if=/dev/zero of=mydisk bs=1024 count=1440
	mke2fs -b 1024 mydisk 1440

init:
	dd if=/dev/zero of=mydisk bs=1024 count=1440
	mke2fs -b 1024 mydisk 1440
