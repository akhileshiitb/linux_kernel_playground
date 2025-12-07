#!/bin/bash

# Build necessary userspace programs

## PORT layer ########
GNU_COREUTILS_PATH=coreutils/coreutils-9.9 
GNU_BASH_PATH=shell/bash-5.3 
CROSS_COMPILE=aarch64-linux-gnu
TH_NUM=64
INITRAMFS_PATH=$(realpath initramfs_kekhale)
COREUTIL_INSTALL_PATH=${INITRAMFS_PATH}
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

	make distclean
	./configure   --host=${CROSS_COMPILE} --prefix=${COREUTIL_INSTALL_PATH} --enable-no-install-program=stdbuf gl_cv_func_working_mktime=yes LDFLAGS="-static -pthread -Wl,--allow-multiple-definition"
	make all -j${TH_NUM}
	
	pr_banner_end
	popd

}

# function to build BASH shell
function build_bash() {
	pushd ${GNU_BASH_PATH}
	pr_banner "Building bash shell"

	make distclean
	./configure --host=${CROSS_COMPILE} --enable-static-link
	make all -j${TH_NUM}

	pr_banner_end
	popd

}

# build appds from ./apps directory
function build_apps() {
	pushd apps
	for app in *.c
	do
		echo "Building app ${app}"
		APP_OUT=$(echo ${app} | awk -F. '{ print $1 }')
		${CROSS_COMPILE}-gcc -static -o ${APP_OUT} ${app}
	done
	popd

}

function install_programs() {


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
}

function do_initramfs() {
	# copy init script to /init
	cp init_script ${INITRAMFS_PATH}/init
	
	# generate initramfs cpio archive for linux
	pushd ${INITRAMFS_PATH}
	find . | cpio -o -H newc > ../initramfs.cpio
	popd
}

# main 
#
# clear out initramfs
rm -rf initramfs_kekhale
mkdir -p initramfs_kekhale

build_bash

build_coreutils

build_apps

install_programs

do_initramfs

