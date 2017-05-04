#!/bin/sh

HOST='q4legkpasqmkb51k.myfritz.net'
USER='root'
PASSWD='md8yz92'
FILE='kernel'

ftp -n $HOST <<END_SCRIPT
quote USER $USER
quote PASS $PASSWD
binary
passive
cd ..
cd tftpboot
put $FILE
quit
END_SCRIPT
exit 0
