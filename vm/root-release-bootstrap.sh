#!/bin/bash

# Print commands and exit on errors
set -xe

export DEBIAN_FRONTEND=noninteractive

apt-get update -q

apt-get install -qq -y --no-install-recommends --fix-missing \
  ca-certificates curl git net-tools python3 python3-pip tcpdump unzip \
  vim wget make gcc libc6-dev flex bison libelf-dev libssl-dev

# Compile and install kernel with P4TC support
git clone https://github.com/p4tc-dev/linux-p4tc-pub.git
cd linux-p4tc-pub/
cp /home/vagrant/config-p4tc-x86-ubuntu-22.04 .config
make olddefconfig
make -j `nproc`
make modules_install && make install
