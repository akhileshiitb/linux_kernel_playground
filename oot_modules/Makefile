obj-y += xband/
obj-y += char_intf/
obj-y += ioctl_intf/
obj-y += sysfs_intf/
obj-y += procfs_intf/
obj-y += block_device/
obj-y += block_io/
obj-y += timers/
obj-y += hrtimers/

# Top-level Makefile for building all kernel modules
# Set KERNEL_DIR to point to your kernel source directory
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

# List of all subdirectories containing modules
SUBDIRS := block_device block_io char_intf hrtimers ioctl_intf procfs_intf sysfs_intf timers xband

# Default target - build all modules
all: $(SUBDIRS)

# Build each module directory
$(SUBDIRS):
	@echo "Building $@..."
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD)/$@ modules

# Clean all modules
clean: $(addprefix clean-,$(SUBDIRS))

clean-%:
	@echo "Cleaning $*..."
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD)/$* clean

# Install all modules (optional)
install: $(addprefix install-,$(SUBDIRS))

install-%:
	@echo "Installing $*..."
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD)/$* modules_install

.PHONY: all clean install $(SUBDIRS) $(addprefix clean-,$(SUBDIRS)) $(addprefix install-,$(SUBDIRS))
