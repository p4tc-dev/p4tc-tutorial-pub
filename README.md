# P4TC tutorial

Welcome to the P4TC tutorial! This repository will help you get started with the P4TC project.

Before you start, [install required software](#required-software). Also, you may want to visit [p4tc.dev](https://www.p4tc.dev/) to learn more about the P4TC project.

## Exercises

TO DO

1. Basics

## Required software

Before you start, you have to build your own virtual machine, where you will run P4TC exercises.

First, install [Vagrant](https://vagrantup.com) and [VirtualBox](https://virtualbox.org) on your OS.

You also need to install the following Vagrant plugin:

```bash
vagrant plugin install vagrant-disksize
```

Then, clone this repository and bootstrap the VM:

```bash
git clone https://github.com/p4tc-dev/p4tc-tutorial.git
cd p4tc-tutorial/vm
vagrant up
```

The Vagrant scripts will install all the required software (including the Linux kernel with P4TC support, iproute2, p4c) 
and download this repository to the VM under `/home/p4tc/p4tc-tutorial`. At the end of script's execution, the VM will be rebooted to apply the new configuration. 
You might need to wait a few minutes before you will be able to log in to the VM. Once the VM is ready, you can SSH to it with:

```bash
ssh -p 2222 p4tc@127.0.0.1
# password: p4tc
```

You can also use VirtualBox GUI to access VM.

# TO DO

- Create a "release" VM flavor that uses .deb package that is built by p4tc CI. The current way of building from sources should be used as a "dev" flavor.
- Allow to specify Git commit SHAs in the "dev" VM for linux kernel, iproute2 and p4c. 

