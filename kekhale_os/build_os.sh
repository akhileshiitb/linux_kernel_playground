#!/bin/bash

# Build necessary userspace programs

## PORT layer ########
GNU_COREUTILS_PATH=coreutils/coreutils-9.9 
GNU_BASH_PATH=shell/bash-5.3 
CROSS_COMPILE=aarch64-linux-gnu
TH_NUM=64
INITRAMFS_PATH=$(realpath initramfs_kekhale)
COREUTIL_INSTALL_PATH=${INITRAMFS_PATH}
KERNEL_SRC=linux/linux
KERNEL=${KERNEL_SRC}/arch/arm64/boot/Image
CLEAN=$1
LIBRARY=/usr/${CROSS_COMPILE}/lib
#######################

function pr_banner() {
	echo "############ $1 ###################"
}

function pr_banner_end() {
	echo "###################################"
}

# function to build GNU coreutils
function build_coreutils() {
	pushd ${GNU_COREUTILS_PATH}
	pr_banner "Building coreutils"

	if [ "${CLEAN}" = "c" ]; then
		make distclean
		./configure   --host=${CROSS_COMPILE} --prefix=${COREUTIL_INSTALL_PATH}
	fi

	make all -j${TH_NUM}
	
	pr_banner_end
	popd

}

# function to build BASH shell
function build_bash() {
	pushd ${GNU_BASH_PATH}
	pr_banner "Building bash shell"

	if [ "${CLEAN}" = "c" ]; then
		make distclean
		./configure --host=${CROSS_COMPILE} --enable-static-link
	fi
	make all -j${TH_NUM}

	pr_banner_end
	popd

}

# build appds from ./apps directory
function build_apps() {
	pr_banner "Building Apps"
	pushd apps
	for app in *.c
	do
		echo "Building app ${app}"
		APP_OUT=$(echo ${app} | awk -F. '{ print $1 }')
		${CROSS_COMPILE}-gcc -static -o ${APP_OUT} ${app}
	done
	popd
	pr_banner_end

}

function install_programs() {

	pr_banner "Installing programs"

	# get coreutils
	pushd ${GNU_COREUTILS_PATH}
	make install
	popd

	# get bash shell
	cp ${GNU_BASH_PATH}/bash ${INITRAMFS_PATH}/bin/

	# install apps
	mkdir -p ${INITRAMFS_PATH}/usr/bin
	cp apps/* ${INITRAMFS_PATH}/usr/bin

	# create VFS directories
	mkdir -p ${INITRAMFS_PATH}/proc
	mkdir -p ${INITRAMFS_PATH}/sys

	# create library folder to keep linker and libraries
	mkdir -p ${INITRAMFS_PATH}/lib
	# install shared libraries including linker/loader
	cp ${LIBRARY}/* ${INITRAMFS_PATH}/lib/

	pr_banner_end
}

function do_initramfs() {
	# copy init script to /init
	cp init_script ${INITRAMFS_PATH}/init
	
	# generate initramfs cpio archive for linux
	pushd ${INITRAMFS_PATH}
	find . | cpio -o -H newc > ../initramfs.cpio
	popd
}

# function to boot distribution in qemu
function boot_qemu() {
	qemu-system-aarch64 -machine virt -cpu cortex-a53 -kernel ${KERNEL} -m 1G -initrd initramfs.cpio -nographic
}

# function to compile the linux kernel
function compile_kernel() {
	pushd ${KERNEL_SRC}

	if [ "${CLEAN}" = "c" ]
	then
		make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE}- distclean
		make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE}- defconfig
	fi

	make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE}- all -j${TH_NUM}
	popd
}

# main 
#
# clear out initramfs
rm -rf initramfs_kekhale
mkdir -p initramfs_kekhale

compile_kernel

build_bash

build_coreutils

build_apps

install_programs

do_initramfs

if [ $? = 0 ]
then
	boot_qemu
fi


