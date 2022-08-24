kernel-version=5.15.62
arch=x86_64

circle_build_kernel:
	git clone -b v$(kernel-version) --single-branch --depth 1 git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
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
	cd linux-stable && $(MAKE) clean
	cd linux-stable && $(MAKE) mrproper

btf:
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
	cp -f include/kern/vmlinux.h .circleci/_vmlinux.h

btf_circle:
	cp -f .circleci/_vmlinux.h vmlinux.h

track:
	clang -O2 -Wall \
	-target bpf -g -c track.c -o track.o

user:
	clang -Wall user.c -o user.o -c
	clang -o user user.o -lbpf

skel:
	bpftool gen skeleton track.o > track.skel.h

all: track skel user
