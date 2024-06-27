#!/bin/bash

# Print commands and exit on errors
set -xe

export DEBIAN_FRONTEND=noninteractive

apt-get update -q

apt-get install -qq -y --no-install-recommends --fix-missing \
  ca-certificates curl git net-tools python3 python3-pip jq tcpdump unzip \
  vim wget make gcc libc6-dev flex bison libelf-dev libssl-dev dpkg-dev build-essential debhelper \
  pkg-config cmake autoconf automake libtool g++ \
  libfl-dev libgc-dev gcc-multilib libmnl-dev

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

wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
sudo ./llvm.sh 19

sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-19 100
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100
sudo update-alternatives --install /usr/bin/clang-cpp clang-cpp /usr/bin/clang-cpp-19 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100
sudo update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-19 100

sudo ln -s /usr/bin/llvm-ar-19 /usr/bin/llvm-ar
sudo ln -s /usr/bin/llvm-as-19 /usr/bin/llvm-as
sudo ln -s /usr/bin/llvm-bcanalyzer-19 /usr/bin/llvm-bcanalyzer
sudo ln -s /usr/bin/llvm-cov-19 /usr/bin/llvm-cov
sudo ln -s /usr/bin/llvm-diff-19 /usr/bin/llvm-diff
sudo ln -s /usr/bin/llvm-dis-19 /usr/bin/llvm-dis
sudo ln -s /usr/bin/llvm-extract-19 /usr/bin/llvm-extract
sudo ln -s /usr/bin/llvm-link-19 /usr/bin/llvm-link
sudo ln -s /usr/bin/llvm-mc-19 /usr/bin/llvm-mc
sudo ln -s /usr/bin/llvm-nm-19 /usr/bin/llvm-nm
sudo ln -s /usr/bin/llvm-objdump-19 /usr/bin/llvm-objdump
sudo ln -s /usr/bin/llvm-ranlib-19 /usr/bin/llvm-ranlib
sudo ln -s /usr/bin/llvm-readobj-19 /usr/bin/llvm-readobj
sudo ln -s /usr/bin/llvm-rtdyld-19 /usr/bin/llvm-rtdyld
sudo ln -s /usr/bin/llvm-size-19 /usr/bin/llvm-size
sudo ln -s /usr/bin/llvm-stress-19 /usr/bin/llvm-stress
sudo ln -s /usr/bin/llvm-symbolizer-19 /usr/bin/llvm-symbolizer

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
sudo pip3 install scapy
git clone https://github.com/ebiken/sendpacket

#running depmod
depmod -a
