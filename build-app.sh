#!/bin/bash

set -e

echo Compiling test application...

root=`pwd`
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
$CC -o test test.c

if [ -d ../BareMetal-OS ]; then
	echo "Running test app"
	cp test.app ../BareMetal-OS/sys/
	cd ../BareMetal-OS; APPS=test.app BMFS_SIZE=16 ./baremetal.sh bnr
	cd $root
fi

echo Complete!
