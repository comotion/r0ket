#!/bin/bash
# CONFIG
DEPLOYFIRMWARE="r0ket-df.dd"			# not used in "cp mode"
DEPLOYMODE="cp"					# could be cp
RELEASEDIR="release"

clear
echo "################# /!\ ATTENTION /!\ #################"
echo "im asuming you are in r0ket/ and root!"
echo "furthermore, you copied BOTH udev rules from firmware/bootloader/42-*"
echo "################# /!\ ATTENTION /!\ #################"
echo ""
echo ""
echo ""

if [ $# -gt 0 ]
then
	echo "using \"$1\" for flashing"
	RELEASEDIR="$1"
fi

# change to release dir where all the files from firmware/release-all end up
cd $RELEASEDIR

if [ ! -f "initial.bin" ]
then
	echo "missing: initial.bin"
fi

if [ ! -f "initial.bin.ref" ]
then
	echo "missing: initial.bin.ref"
	echo "please read the initial.bin from a working test-r0ket via:"
	echo "dd if=/dev/sdb of=initial.bin.ref bs=512 seek=4"
fi

if [ ! -f "final.bin" ]
then
	echo "missing: final.bin"
fi

if [ ! -f "$DEPLOYFIRMWARE" ]
then
	echo "missing: $DEPLOYFIRMWARE"
	echo "switching deploymode to: cp"
	DEPLOYMODE="cp"
fi



# unmount old mounts, just to be save
umount /tmp/r0ket 2> /dev/null
mkdir -p /tmp/r0ket 2> /dev/null
rm -rf /tmp/r0ket/* 2> /dev/null

echo -n "waiting for device in ISP mode"
while [ ! -d "/dev/lpcflash" ]
do
	echo -n "."
	sleep 1
done
echo ""

ISPDEV="/dev/"`ls /dev/lpcflash`
echo "device detected as:" $ISPDEV

if [ -f "$ISPDEV" ]
then
	umount $ISPDEV
fi

echo "flashing initial.bin"
dd if=initial.bin of=$ISPDEV bs=512 seek=4
sync
echo "initial.bin flashed"
echo "verifying initial.bin"
dd if=$ISPDEV of=initial.bin.test bs=512 seek=4
sync
#DIFFERENCE=`diff initial.bin.ref initial.bin.test`
# for testing
DIFFERENCE=""
if [ "$DIFFERENCE" ]
then
	echo "flashing not successful. flash-hw broken?"
	exit 1;
fi
#rm initial.bin.test
echo "initial.bin verified or not"

echo "-----------------------------------------"
echo "now powercycle the r0ket and press enter"
echo "-----------------------------------------"
read

# sleep so internal vfat can be formatted correctly
sleep 3

echo -n "waiting for device in storage mode"
while [ ! -d "/dev/r0ketflash" ]
do
	echo -n "."
	sleep 1
done
echo ""

DATADEV="/dev/"`ls /dev/r0ketflash`
echo "device detected as:" $DATADEV

echo "deploymode: $DEPLOYMODE"

if [ $DEPLOYMODE == "dd" ]
then
	if [ -f "$DATADEV" ]
	then
		umount $DATADEV
	fi
	
	echo "flashing $DEPLOYFIRMWARE"
	dd if=$DEPLOYFIRMWARE of=$DATADEV
	sync
	echo "$DEPLOYFIRMWARE flashed"
elif [ $DEPLOYMODE == "cp" ]
then
	echo "mounting device to /tmp/r0ket"
	mount -t vfat $DATADEV /tmp/r0ket
	
	echo "generating keys"
	../tools/crypto/generate-keys

	rm -rf /tmp/r0ket/*
	
	echo "copying all files to dataflash"
	cp -r files/* /tmp/r0ket/
	
	echo "touching flashed.cfg"
	touch /tmp/r0ket/flashed.cfg
	sync
	sleep 1
	umount /tmp/r0ket
	
	echo "dataflashing done"
else
	echo "deploymode not recognised"
	read
	exit
fi

# left press might not be needed since the r0ket goes into ISP on its own
echo "-----------------------------------------"
#echo "press the 5-way left for about 2 sec (or not) - on pc press enter"
echo "device should be in ISP mode already"
echo "-----------------------------------------"
#read
sleep 2

echo -n "waiting for device in ISP mode"
while [ ! -d "/dev/lpcflash" ]
do
	echo -n "."
	sleep 1
done
echo ""

ISPDEV="/dev/"`ls /dev/lpcflash`
echo "device detected as:" $ISPDEV

# umount once again, just to be sure
if [ -f "$ISPDEV" ]
then
	umount $ISPDEV
fi

echo "flashing final.bin"
dd if=final.bin of=$ISPDEV bs=512 seek=4
sync
echo "final.bin flashed"

echo "all done. powercycle once again. firmware should boot."