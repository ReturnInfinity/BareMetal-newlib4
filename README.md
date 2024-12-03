# BareMetal-newlib4

based on [BareMetal-newlib](https://github.com/ReturnInfinity/BareMetal-newlib)

This port was created by [effbiae](https://github.com/effbiae)

Introduction
------------

This repository contains the files, script, and instructions necessary to build the [newlib](http://sourceware.org/newlib/) C library for BareMetal OS. The latest version of Newlib as of this writing is 4.4.0.20231231

newlib gives BareMetal OS access to the standard set of C library calls like `printf()`, `scanf()`, `memcpy()`, etc.

These instructions are for executing on a 64-bit Linux host. Building on a 64-bit host saves us from the steps of building a cross compiler. The latest distribution of Ubuntu was used while writing this document.


Building Details
----------------

You will need the following Linux packages. Use your prefered package manager to install them:

	libtool sed gawk bison flex m4 texinfo texi2html unzip make

You also need exact versions of the following:
 * autoconf - 2.69
 * automake - 1.15.1

On a Debian-based system this can be accomplished via the following:
```
# Remove software if already installed
sudo apt remove autoconf automake

# Download the required versions 
wget https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
wget https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz

# Extract them
tar -xvzf autoconf-2.69.tar.gz
tar -xvzf automake-1.15.1.tar.gz

# Build and install them
cd autoconf-2.69
./configure
make
sudo make install
cd ..
cd automake-1.15.1
./configure
make
sudo make install
cd ..

# Adjust the path variable
PATH=$PATH:/usr/local/bin
```

Run the build script to download, patch and build:

	./build-newlib.sh

After a lengthy compile you should have newlib installed in ./lib, headers in ./include and `test.app` built.

`./output/x86_64-pc-baremetal/lib/libc.a` is the compiled C library that is ready for linking. 
`./output/x86_64-pc-baremetal/lib/crt0.o` is the starting binary stub for your program.

`./build-newlib.sh` compiles `test.app` as

	gcc -I include -c test.c -o test.o
	ld -T app.ld -o test lib/crt0.o test.o lib/libc.a
	objcopy -O binary test test.app

to run `test.app`, copy it to your `BareMetal-OS/sys` directory and run

	APPS=test.app BMFS_SIZE=32 ./baremetal.sh bnr

By default libc.a will be about 6.4 MiB. You can `strip` it to make it a little more compact. `strip` can decrease it to about 1.4 MiB.

	strip --strip-debug lib/libc.a
