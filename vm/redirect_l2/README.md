# redirect_l2

The *redirect_l2* program first parses ethernet <u>ipv4</u> packets. Any other packets are
rejected. After the parser recognizes an ipv4 packet, the src ip address is used as a lookup
key for table *nh_table*. On a table hit, the programmed action *send_nh(srcmac, dstmac, port)* instance is executed. The action first sets the src and destination mac address and then redirects the packet to a specified port. On a table miss the packet is simply dropped.

This directory also contains the `api-lifetime.md` which should provide a description of
all necessary API calls you will need for these first steps.

## Breakdown

The `redirect_l2.c` program demonstrates basic CRUD (Create, Read, Update, Delete) operations on a P4TC table by performing the following steps in order:
1. **CREATE**: Provisions the datapath and creates 3 entries in `ingress/nh_table` with action `ingress/send_nh`:
   - Key: `192.168.1.10` (dev: `port0`, dmac: `00:AA:BB:CC:DD:EE`, smac: `00:11:22:33:44:55`)
   - Key: `10.0.0.5` (dev: `port0`, dmac: `00:22:33:44:55:66`, smac: `00:AA:BB:CC:DD:FF`)
   - Key: `172.16.0.100` (dev: `port0`, dmac: `00:DE:AD:BE:EF:00`, smac: `00:CA:FE:BA:BE:01`)
2. **READ**: Performs a targeted read to check the initial state of the entry with key `10.0.0.5`.
3. **UPDATE**: Updates the action parameters for the entry with key `10.0.0.5` (new dmac: `FF:FF:FF:FF:FF:FF`, smac: `00:00:00:00:00:00`).
4. **READ**: Performs a targeted read to verify the update on key `10.0.0.5`.
5. **DELETE**: Deletes the entry with key `172.16.0.100`.
6. **READ**: Performs a full table dump.
7. **CLEANUP**: Flushes the entire table.
8. **FINAL CHECK**: Reads the table again to verify that it is empty.

Below is a step by step describing how to run this simple example

## Setup

`sudo /home/vagrant/p4tc-examples-pub/build-simple-p4`

## Running

Enter the container p4node:

`sudo ip netns exec p4node /bin/bash`

In the directory where this README is compile:

`make`

To execute the compile program, simply run:

`./redirect_l2`
