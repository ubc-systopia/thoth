btf:
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h

track:
	clang -O2 -Wall \
	-target bpf -g -c track.c -o track.o

user: 
	clang -Wall user.c -o user.o -c
	clang -o user user.o -lbpf 
	

skel:
	bpftool gen skeleton track.o > track.skel.h

all:
	track skel user
