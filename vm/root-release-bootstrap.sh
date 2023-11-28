#!/bin/bash

# Print commands and exit on errors
set -xe

export DEBIAN_FRONTEND=noninteractive

apt-get update -q

apt-get install -qq -y --no-install-recommends --fix-missing \
  ca-certificates curl git net-tools python3 python3-pip jq tcpdump unzip \
  vim wget make gcc libc6-dev flex bison libelf-dev libssl-dev dpkg-dev build-essential debhelper \
  pkg-config cmake autoconf automake libtool g++ libboost-dev libboost-iostreams-dev libboost-graph-dev \
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

# Download and install protobuf (required by p4c)
sudo apt-get purge -y python3-protobuf || echo "Failed to remove python3-protobuf, probably because there was no such package installed"
sudo pip3 install protobuf==3.18.1
sudo pip3 install scapy

# Download and compile p4c
cd /home/vagrant/libs/
git clone --recursive https://github.com/p4lang/p4c.git
cd p4c
git submodule update --init --recursive
mkdir -p build
cd build
cmake .. -DENABLE_P4TC=ON -DENABLE_DPDK=OFF
make -j`nproc`
make install