#!/bin/bash

# Build necessary userspace programs

## PORT layer ########
GNU_COREUTILS_PATH=coreutils/coreutils-9.9 
GNU_BASH_PATH=shell/bash-5.3 
CROSS_COMPILE=aarch64-linux-gnu
TH_NUM=64
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

function install_programs() {
	cp ${GNU_BASH_PATH}/bash .
	mv bash init # out init program is bash
}

# main 

build_bash

build_coreutils

install_programs

