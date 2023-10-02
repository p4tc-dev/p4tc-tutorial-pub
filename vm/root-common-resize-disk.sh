#!/bin/bash

# Print commands and exit on errors
set -xe

# Resizing disk
# Works for lvm drives and sda3 is specific to bento/ubuntu-22.04
sudo swapoff -a
printf 'Fix\n3\n100%%' | sudo parted /dev/sda resizepart ---pretend-input-tty
sudo pvresize /dev/sda3
sudo lvextend -l +100%FREE /dev/ubuntu-vg/ubuntu-lv
sudo resize2fs /dev/ubuntu-vg/ubuntu-lv