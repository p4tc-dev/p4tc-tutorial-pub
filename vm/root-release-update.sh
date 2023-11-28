#!/bin/bash

# Print commands and exit on errors
set -xe

cd /home/vagrant/kernel
rm ./*
curl -s https://api.github.com/repos/p4tc-dev/linux-p4tc-pub/releases/latest | \
	grep "browser_download_url.*deb" | \
	cut -d : -f 2,3 | \
  	tr -d \" | \
	wget -i -
sudo dpkg -i ./linux-headers-*
sudo dpkg -i ./linux-libc-*
sudo dpkg -i ./linux-image-*

cd /home/vagrant/libs/libbpf
git pull
cd src
BUILD_STATIC_ONLY=y OBJDIR=build DESTDIR=root make install

cd /home/vagrant/libs/iproute2-p4tc-pub
git pull
make && make install

cd /home/vagrant/libs/p4c
git pull
cd build
cmake .. -DENABLE_P4TC=ON -DENABLE_DPDK=OFF -DENABLE_TEST_TOOLS=ON
make -j`nproc`
make install
