# P4TC tutorial

Welcome to the P4TC tutorial! This repository will help you get started with the P4TC project.

Before you start, [install required software](#required-software). Also, you may want to visit [p4tc.dev](https://www.p4tc.dev/) to learn more about the P4TC project.

<!---
## Exercises

TO DO

1. Basics
-->

## Required software

Before you start, you have to build your own virtual machine, where you will run P4TC exercises.

First, install [Vagrant](https://vagrantup.com) and [VirtualBox](https://virtualbox.org) on your OS.

You also need to install the following Vagrant plugin:

```bash
vagrant plugin install vagrant-disksize
vagrant plugin install vagrant-reload
```

There are currently three flavors of the VM.

### Examples flavor
The "examples" flavor will download and install every part of the p4tc infrastructure that is compiled as a .deb file and compile and install libbpf and iproute2. The examples flavor is the default.
The "examples" flavor is intended for people interested in running p4tc examples. The p4c compiler is not available in this flavor.

To use the "examples" flavor, clone this repository and bootstrap the VM.
The "examples" flavor should take about 10-20 minutes to setup. 

```bash
git clone https://github.com/p4tc-dev/p4tc-tutorial-pub.git
cd p4tc-tutorial/vm
vagrant up 
```

### Release flavor
The "release" flavor that will download and install every part of the p4tc infrastructure that is compiled as a .deb file and compile and install the rest along with the p4c.
The "release" flavor is intendend for most users that want to compile new p4 programs into p4c and test them.

To use the "release" flavor, clone this repository and bootstrap the VM.
The "release" flavor should take about 10-20 minutes to setup. 

```bash
git clone https://github.com/p4tc-dev/p4tc-tutorial-pub.git
cd p4tc-tutorial/vm
vagrant up release
```

### Dev flavor
The "dev" flavor will download and compile everything from scratch.
The "dev" flavor is intended for people interested in making changes also in kernel.

To use the "dev" flavor, clone this repository and bootstrap the VM.
Be advised that the "dev" flavor will require more time to complete and will take approximately one hour. 

```bash
git clone https://github.com/p4tc-dev/p4tc-tutorial-pub.git
cd p4tc-tutorial/vm
vagrant up dev
```

All flavors will install all the required software per flavor, including the Linux kernel with P4TC support and iproute2 along with downloading this repository to the VM under `/home/p4tc/p4tc-tutorial`.
At the end of script's execution, the VM will be rebooted to apply the new configuration. You might need to wait a few minutes before you will be able to log in to the VM. Once the VM is ready, you can SSH to it with:

```bash
ssh -p 2222 p4tc@127.0.0.1
# password: p4tc
```

or simply if you are already in the directory `vm`:
```bash
vagrant ssh `flavor`
```
e.g.:
```bash
vagrant ssh release
```

You can also use VirtualBox GUI to access VM.

<!---
# TO DO

- Allow to specify Git commit SHAs in the "dev" VM for linux kernel, iproute2 and p4c. 
-->
