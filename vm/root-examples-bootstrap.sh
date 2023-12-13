#!/bin/bash

# Print commands and exit on errors
set -xe

export DEBIAN_FRONTEND=noninteractive

apt-get update -q

apt-get install -qq -y --no-install-recommends --fix-missing \
  ca-certificates curl git net-tools python3 python3-pip jq tcpdump unzip \
  vim wget make gcc libc6-dev flex bison libelf-dev libssl-dev dpkg-dev build-essential debhelper \
  pkg-config cmake autoconf automake libtool g++ \
  libfl-dev libgc-dev llvm clang gcc-multilib libmnl-dev

# Download and install kernel with P4TC support
mkdir -p /home/vagrant/kernel
cd /home/vagrant/kernel
curl -s https://api.github.com/repos/p4tc-dev/linux-p4tc-pub/releases/latest | \
	grep "browser_download_url.*deb" | \
	cut -d : -f 2,3 | \
  	tr -d \" | \
	wget -i -
sudo dpkg -i ./linux-headers-*
sudo dpkg -i ./linux-libc-*
sudo dpkg -i ./linux-image-*

# Download and compile libbpf
mkdir -p /home/vagrant/libs
cd /home/vagrant/libs
git clone https:\//github.com/libbpf/libbpf.git
cd libbpf/src
mkdir build root
BUILD_STATIC_ONLY=y OBJDIR=build DESTDIR=root make install

# Download and compile iproute2
cd /home/vagrant/libs/
git clone https:\//github.com/p4tc-dev/iproute2-p4tc-pub
cd iproute2-p4tc-pub/
\/home/vagrant/libs/iproute2-p4tc-pub/configure --libbpf_dir \/home/vagrant/libs/libbpf/src/root/
make && make install && cp etc/iproute2/p4tc_entities /etc/iproute2 && cp -r etc/iproute2/p4tc_entities.d /etc/iproute2

#get header files for compiling. We should also fix dev and release to have something similar
wget -P /home/vagrant/libs/include https://raw.githubusercontent.com/p4lang/p4c/main/backends/ebpf/runtime/ebpf_kernel.h
wget -P /home/vagrant/libs/include https://raw.githubusercontent.com/p4lang/p4c/main/backends/ebpf/runtime/ebpf_common.h

#get examples
cd /home/vagrant
git clone https://github.com/p4tc-dev/p4tc-examples-pub.git

#get sendpacket
cd /home/vagrant
git clone https://github.com/ebiken/sendpacket

#running depmod
depmod -a
