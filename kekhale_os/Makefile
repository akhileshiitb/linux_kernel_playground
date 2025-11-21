
KERNEL := ../linux/arch/arm64/boot/Image 

all: userspace initramfs

userspace:
	aarch64-linux-gnu-gcc -static -o init init.c
	aarch64-linux-gnu-gcc -static -o program1 program1.c

initramfs: initramfs_files.txt
	cpio -o -H newc < initramfs_files.txt  > initramfs.cpio

clean:
	rm -rf initramfs.cpio init program1

boot:
	#qemu-system-aarch64 -machine virt,gic-version=3 -cpu cortex-a57 -smp 4 -m 1G -kernel ../../linux-next/arch/arm64/boot/Image -initrd initramfs.cpio -nographic -append "console=ttyAMA0 earlyprintk=ttyAMA0 root=/dev/ram0 rw" 
	#qemu-system-aarch64 -machine virt -cpu cortex-a72 -kernel ../../linux-next/arch/arm64/boot/Image -initrd initramfs.cpio -nographic -append "console=ttyAMA0,115200 earlyprintk=ttyAMA0" 
	qemu-system-aarch64 -machine virt -cpu cortex-a53 -kernel $(KERNEL) -initrd initramfs.cpio -nographic
