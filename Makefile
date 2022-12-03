arch=x86_64
version=0.1.0

export ARCH=x86

circle_build_kernel:
	git clone -b linux-rolling-stable --single-branch --depth 1 git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
	cd linux-stable && rm -rf .git
	cd linux-stable && $(MAKE) olddefconfig
	cd linux-stable && sed -i -e "s/CONFIG_LSM=\"yama,loadpin,safesetid,integrity,selinux,smack,tomoyo,apparmor\"/CONFIG_LSM=\"yama,loadpin,safesetid,integrity,selinux,smack,tomoyo,apparmor,bpf\"/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_BPF_LSM is not set/CONFIG_BPF_LSM=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_IP_ADVANCED_ROUTER is not set/CONFIG_IP_ADVANCED_ROUTER=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_IP_MULTIPLE_TABLES is not set/CONFIG_IP_MULTIPLE_TABLES=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NETFILTER_NETLINK is not set/CONFIG_NETFILTER_NETLINK=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NETFILTER_NETLINK_QUEUE is not set/CONFIG_NETFILTER_NETLINK_QUEUE=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NETFILTER_NETLINK_ACCT is not set/CONFIG_NETFILTER_NETLINK_ACCT=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NETFILTER_NETLINK_LOG is not set/CONFIG_NETFILTER_NETLINK_LOG=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NF_CT_NETLINK is not set/CONFIG_NF_CT_NETLINK=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NETFILTER_XT_TARGET_MARK is not set/CONFIG_NETFILTER_XT_TARGET_MARK=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NET_SCHED is not set/CONFIG_NET_SCHED=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NET_SCH_INGRESS is not set/CONFIG_NET_SCH_INGRESS=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_SCSI_NETLINK is not set/CONFIG_SCSI_NETLINK=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_SCSI_NETLINK is not set/CONFIG_SCSI_NETLINK=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_SCSI_FC_ATTRS is not set/CONFIG_SCSI_FC_ATTRS=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_SCSI is not set/CONFIG_SCSI=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NF_CONNTRACK is not set/CONFIG_NF_CONNTRACK=y/g" .config
	cd linux-stable && sed -i -e "s/# CONFIG_NETFILTER_XT_MARK is not set/CONFIG_NETFILTER_XT_MARK=y/g" .config
	cd linux-stable && echo "CONFIG_NF_CT_NETLINK=y" >> .config
	cd linux-stable && echo "CONFIG_SCSI_NETLINK=y" >> .config
	cd linux-stable && $(MAKE) -j16 ARCH=${arch}
	cd linux-stable && $(MAKE) headers_install ARCH=${arch} INSTALL_HDR_PATH=/usr
	cd linux-stable && $(MAKE) modules_install ARCH=${arch}
	cd linux-stable && $(MAKE) install ARCH=${arch}
	cd linux-stable/tools/lib/bpf && $(MAKE) all
	cd linux-stable/tools/bpf/resolve_btfids && sudo $(MAKE) all
	cd linux-stable/tools/bpf/bpftool && sudo $(MAKE) all
	cd linux-stable/tools/lib/bpf && sudo $(MAKE) install
	cd linux-stable/tools/bpf/bpftool && sudo $(MAKE) install
	cd linux-stable && $(MAKE) clean
	cd linux-stable && $(MAKE) mrproper

btf:
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
	cp -f vmlinux.h .circleci/_vmlinux.h

btf_circle:
	cp -f .circleci/_vmlinux.h vmlinux.h

track:
	clang -O2 -Wall \
	-target bpf -g -c track.c -o track.o

user:
	clang -Wall user.c -o user.o -c
	clang -o thothd \
	user.o \
	-l:libbpf.so.0 -lpthread

cli:
	clang -Wall thoth.c -o thoth.o -c
	clang -o thoth \
	thoth.c \
	-lpthread

skel:
	bpftool gen skeleton track.o > track.skel.h

uncrustify:
	uncrustify -c uncrustify.cfg --replace record.h
	uncrustify -c uncrustify.cfg --replace spade.c
	uncrustify -c uncrustify.cfg --replace track.c
	uncrustify -c uncrustify.cfg --replace user.c
	uncrustify -c uncrustify.cfg --replace common.h
	uncrustify -c uncrustify.cfg --replace thoth.c

uncrustify_clean:
	rm *backup*~

all: track skel user cli uncrustify uncrustify_clean

install:
	sudo cp --force ./thothd /usr/bin/thothd
	sudo cp --force ./thothd.service /etc/systemd/system/thothd.service
	sudo systemctl enable thothd.service

start:
	sudo systemctl start thothd.service

stop:
	sudo systemctl stop thothd.service

status:
	sudo systemctl status thothd.service

uninstall:
	sudo systemctl stop thothd.service
	sudo systemctl disable thothd.service
	sudo rm -f /usr/bin/thothd
	sudo rm -f /etc/systemd/system/thothd.service

rpm: all
	mkdir -p ~/rpmbuild/{RPMS,SRMS,BUILD,SOURCES,SPECS,tmp}
	rpmbuild -bb thoth.spec
	mkdir -p output
	cp ~/rpmbuild/RPMS/x86_64/* ./output

deb:
	sudo alien output/thoth-$(version)-1.x86_64.rpm
	cp *.deb ./output
