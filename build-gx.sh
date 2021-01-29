#!/bin/sh

# Install toolchain for C-SKY:
# Ubuntu/Debian
# sudo add-apt-repository ppa:nationalchip-repo/gxtools
# sudo apt-get update
# sudo apt-get install csky-linux-tools-uclibc-2.8.01
# Manually:
# wget https://launchpad.net/~nationalchip-repo/+archive/ubuntu/gxtools/+sourcefiles/csky-linux-tools-uclibc-2.8.01/1.0.0.0/csky-linux-tools-uclibc-2.8.01_1.0.0.0.tar.xz
#

export PATH=/storage/Projects/hellobox/csky-linux-tools-uclibc-20170724/bin:$PATH
export LANG=C
VAR=""

./configure --host=csky-linux --enable-gxapi --enable-static --disable-satipc

make clean
make

csky-linux-strip minisatip
