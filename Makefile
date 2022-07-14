btf:
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h

kern:
	clang -O2 -Wall \
	-g -c track.c -o track.o

user: 
	clang -Wall user.c -o user.o -c

skel:
	bpftool gen skeleton track.o > track.skel.h

all:
	btf kern skel
