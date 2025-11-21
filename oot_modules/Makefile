
# Top-level Makefile for building all kernel modules
# Set KERNEL_DIR to point to your kernel source directory
KERNEL_DIR ?= ../linux 

# List of all subdirectories containing modules
SUBDIRS := block_device block_io char_intf hrtimers ioctl_intf procfs_intf sysfs_intf timers xband

CROSS := aarch64-linux-gnu-

# Default target - build all modules
all: $(SUBDIRS)

# Build each module directory
$(SUBDIRS):
	@echo "Building $@..."
	$(MAKE) ARCH=arm64 CROSS_COMPILE=$(CROSS) -C $(KERNEL_DIR) M=$(PWD)/$@ modules

# Clean all modules
clean: $(addprefix clean-,$(SUBDIRS))

clean-%:
	@echo "Cleaning $*..."
	$(MAKE) ARCH=arm64 CROSS_COMPILE=$(CROSS) -C $(KERNEL_DIR) M=$(PWD)/$* clean


.PHONY: all clean $(SUBDIRS) $(addprefix build-,$(SUBDIRS))
