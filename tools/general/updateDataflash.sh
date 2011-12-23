#!/bin/bash

if [ ! -d firmware ] ; then
	echo "Running from wrong directory."
	exit 1
fi

echo -n "waiting for device in storage mode"
while [ ! -d "/dev/r0ketflash" ]
do
	echo -n "."
	sleep 1
done
echo ""

DATADEV="/dev/"`ls /dev/r0ketflash`
echo "device detected as:" $DATADEV

sudo mount $DATADEV /tmp/r0ket/
cd firmware/l0dables
cp *.bin *.int *.c0d *.nik /tmp/r0ket/
sync
sudo umount $DATADEV
