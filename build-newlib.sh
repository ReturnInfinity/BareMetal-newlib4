#!/bin/bash

set -e

opts=`getopt -o v:dn --long newlib-version:,diff,nopatch -- "$@"`

eval set -- "$opts"

ver=4.4.0.20231231  #an ugly version number, but there's no 4.4.0 and this is the latest release
root=`pwd`
target=x86_64-pc-baremetal
diff=false
patch=true

while true; do
	case "$1" in
		-v | --newlib-version) ver="$2"; shift 2;;
		-d | --diff) diff=true; shift;;
		-n | --nopatch) patch=false; shift;;
		-- ) shift; break;;
		* ) break;;
	esac
done

pkg=newlib-$ver
tar=$pkg.tar.gz

CFLAGS_FOR_TARGET="${CFLAGS_FOR_TARGET} -mno-red-zone -mcmodel=large"
CFLAGS_FOR_TARGET="${CFLAGS_FOR_TARGET} -fomit-frame-pointer"
CFLAGS_FOR_TARGET="${CFLAGS_FOR_TARGET} -g"

#if we have ;libc.a then skip getting and building libc.a
if [ ! -e lib/libc.a ]; then
	if [ ! -e $tar ]; then
		echo Downloading newlib
		wget ftp://sourceware.org/pub/newlib/$tar
	fi
	
	if $diff ; then
		rm -rf $pkg
	fi
	
	if [ ! -e $pkg ]; then
		echo Extracting newlib
		tar xf $tar
	fi
	

	diffs="$diffs config.sub"
	diffs="$diffs newlib/configure.host"
	diffs="$diffs newlib/libc/sys/Makefile.inc"
	diffs="$diffs newlib/libc/acinclude.m4"

	if $diff; then
		echo Generating diffs
		for x in $diffs; do
			mkdir -p patches/`dirname $x`
			diff -u $pkg/$x newlib-patched/$x >patches/$x.patch || true
		done
	fi
	
	if $patch; then
		for x in $diffs; do
			(cd $pkg/`dirname $x`; patch <$root/patches/$x.patch)
		done
		cp -r patches/baremetal $pkg/newlib/libc/sys
	fi
	
	echo Configuring newlib
	(cd $pkg/newlib && autoreconf)
	
	echo configuring
	rm -rf tmp/*; mkdir -p tmp; cd tmp
	export CFLAGS_FOR_TARGET
	../$pkg/configure --target=$target --disable-multilib --prefix=$root/output
	
	sed -i "s/TARGET=$target-/TARGET=/g" Makefile
	sed -i "s/WRAPPER) $target-/WRAPPER) /g" Makefile
	
	echo making
	make -j
	make install

	cd $root
	echo installing lib and include to .
	rm -rf lib include
	mv output/$target/lib .
	mv output/$target/include .
fi

echo Compiling test application...

CC=gcc
LD=ld
OBJCOPY=objcopy

CFLAGS="${CFLAGS_FOR_TARGET}  -fno-stack-protector -I $root/include"
LDFLAGS="${LDFLAGS} -T $root/app.ld -z max-page-size=0x1000 -L $root/lib"

echo "$CFLAGS" >CFLAGS
echo "$LDFLAGS">LDFLAGS
$CC $CFLAGS -c test.c
$LD $LDFLAGS -o test lib/crt0.o test.o -lc
$OBJCOPY -O binary test test.app

if [ -d ../BareMetal-OS ]; then
	echo running test app
	cd ../BareMetal-OS; APPS=test.app BMFS_SIZE=16 ./baremetal.sh bnr
	cd $root
fi

echo Complete!

