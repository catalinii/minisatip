#!/bin/sh
cd /home
#clone new folder
git clone https://github.com/dnmnhat/minisatip.git --no-check-certificate
# move to new folder
cd minisatip/
# configure minisatip
./configure make
# build minisatip
make
# start minisat with parameter
sudo ./minisatip -k
