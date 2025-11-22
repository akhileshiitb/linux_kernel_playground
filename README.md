# linux_kernel_playground
Fundamental codes while understanding linux kernel

- Few modules Added 

# how to add out of tree modeules in buildroot. 
1) build oot modules
2) install those modules in rootfs overlay.
3) Configure buildeoot ROOTFS_OVERLAY config and point to where is is locates. 
Note: both kernel version should exactly match and directory structure of overlay should also match
