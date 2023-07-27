#!/bin/bash

# Print commands and exit on errors
set -xe

useradd -m -d /home/p4tc -s /bin/bash p4tc
echo "p4tc:p4tc" | chpasswd

echo "p4tc ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/99_p4tc
chmod 440 /etc/sudoers.d/99_p4tc
usermod -aG vboxsf p4tc
