
KERNEL := ../linux/arch/arm64/boot/Image 
COREUTIL_BINS := /home/akhipati/my_work/source_code/linux_kernel_playground/kekhale_os/coreutils_bins


all: userspace initramfs coreutils

userspace:
	#aarch64-linux-gnu-gcc -static -o init init.c
	aarch64-linux-gnu-gcc -static -o program1 program1.c

coreutils:
	pushd coreutils/coreutils-9.9
	make distclean
	./configure   --host=aarch64-linux-gnu   --prefix=$(COREUTIL_BINS)  --enable-no-install-program=stdbuf   gl_cv_func_working_mktime=yes   LDFLAGS="-static -pthread -Wl,--allow-multiple-definition"
	make all -j32
	popd

shell:
	pushd
	cd shell/bash-5.3
	make distclean
	./configure --host=aarch64-linu-gnu
	make all -j32
	popd

initramfs: initramfs_files.txt
	cpio -o -H newc < initramfs_files.txt  > initramfs.cpio

clean:
	rm -rf initramfs.cpio init program1

boot:
	#qemu-system-aarch64 -machine virt,gic-version=3 -cpu cortex-a57 -smp 4 -m 1G -kernel ../../linux-next/arch/arm64/boot/Image -initrd initramfs.cpio -nographic -append "console=ttyAMA0 earlyprintk=ttyAMA0 root=/dev/ram0 rw" 
	#qemu-system-aarch64 -machine virt -cpu cortex-a72 -kernel ../../linux-next/arch/arm64/boot/Image -initrd initramfs.cpio -nographic -append "console=ttyAMA0,115200 earlyprintk=ttyAMA0" 
	qemu-system-aarch64 -machine virt -cpu cortex-a53 -kernel $(KERNEL) -m 1G -initrd initramfs.cpio -nographic
