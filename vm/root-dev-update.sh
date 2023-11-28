#!/bin/bash

# Print commands and exit on errors
set -xe

cd /home/vagrant/linux-p4tc-pub
git pull
make olddefconfig
make -j`nproc`
make modules_install && make install

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
