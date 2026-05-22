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

pushd /home/vagrant/
sudo dpkg -i ./linux-headers-*
sudo dpkg -i ./linux-libc-*
sudo dpkg -i ./linux-image-*
popd

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
apt-get install python3-scapy -qq -y
git clone https://github.com/ebiken/sendpacket

# Update and install Docker if not present
if ! command -v docker &> /dev/null; then
    curl -fsSL https://get.docker.com -o get-docker.sh
    sh get-docker.sh
    # Add the 'vagrant' user to the docker group so you don't need 'sudo'
    usermod -aG docker vagrant
fi

# 2. Always pull the latest p4c image
echo "Pulling latest P4C image..."
docker pull antoninbas/p4c-lite:latest

# 3. Create a 'p4c' wrapper script
# This makes 'p4c' act like a native command inside the VM
cat <<EOF > /usr/local/bin/p4c-pna-p4tc
#!/bin/bash
# Mount the current directory into the container
docker run --rm -v \$(pwd):/workdir -w /workdir antoninbas/p4c-lite:latest p4c-pna-p4tc "\$@"
EOF

# Make the wrapper executable
chmod +x /usr/local/bin/p4c-pna-p4tc

chown vagrant:vagrant -R p4tc-examples-pub

pushd /home/vagrant/
sudo dpkg -i p4tc-ctrl-runt-api_0.1.0_amd64.deb
popd

#running depmod
depmod -a
