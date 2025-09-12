#!/bin/bash

set -e

# Confirmation prompt
echo "This script will remove the currently installed autoconf and automake, download and install autoconf-2.69 and automake-1.15.1, and adjust the path variable. sudo will be used. Are you sure? Type 'yes' to continue."
read -r response
if [ "$response" != "yes" ]; then
    echo "Aborting..."
    exit 1
fi

# Remove autoconf and automake
sudo apt remove autoconf automake

# Download and autoconf-2.69 and automake-1.15.1
if [ -x "$(command -v curl)" ]; then
	curl -s -O https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
	curl -s -O https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz
else
	wget -q https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
	wget -q https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz

fi

# Extract autoconf-2.69 and automake-1.15.1
tar -xvzf autoconf-2.69.tar.gz
tar -xvzf automake-1.15.1.tar.gz

# Build and install autoconf-2.69 and automake-1.15.1
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

# Set path variable
PATH=$PATH:/usr/local/bin
