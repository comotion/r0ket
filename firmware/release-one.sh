#!/bin/sh -e
# modified to generate one single l0dable and start it right after boot

L0DABLE="1boot"
# TODO: remove me
L0DABLE="avr"

if [ $# -gt 0 ]
then
	L0DABLE="$1"
fi

echo "using \"$L0DABLE\" for image"

TARG=../release-$L0DABLE

if [ ! -d ../firmware ] ; then
	echo Running from wrong directory.
	exit 1
fi

cd ../firmware

if [ -d $TARG ] ; then
	echo Release dir already exists
	echo please remove/move away
	exit 1
fi

if [ ! -f SECRETS.release ] ; then
	echo Please create a SECRETS.release file
	echo containing the release keys.
    	echo
    	echo 'Do not commit them!'
	exit 1
fi

cp SECRETS.release SECRETS

# patch SECRETS to not encrypt stuff
# this release file is checking for the string, filesystem/execute.c is checking for the definition
sed -i 's/#define ENCRYPT_L0DABLE 1/\/\/#define ENCRYPT_L0DABLE 0/g' SECRETS

mkdir -p $TARG/files

echo "###"
echo "### Building initial"
echo "###"

git checkout filesystem/ffconf.h
make clean
make APP=initial
cp firmware.bin $TARG/initial.bin

echo "###"
echo "### Building final"
echo "###"
export FINAL=y
cp filesystem/ffconf.h-final filesystem/ffconf.h
make clean 
./l0dable/mktable.pl
make APP=final
cp firmware.elf $TARG/final.elf
cp firmware.bin $TARG/final.bin


echo "###"
echo "### Building crypto"
echo "###"
(cd ../tools/crypto && make)


echo "###"
echo "### Gathering specific l0dable and placing it as 1boot.int"
echo "###"
# XXX: add crypto stuff
(cd l0dable && make)
if [ ! -f "l0dable/$L0DABLE.bin" ] ; then
	echo "missing l0dable/$L0DABLE.bin"
	exit 1
fi

cp "l0dable/$L0DABLE.bin" $TARG/files/1boot.int
if [ -f "l0dable/$L0DABLE.nik" ] ; then
	cp l0dable/$L0DABLE.nik $TARG/files/
fi
if [ -f "l0dable/$L0DABLE.lcd" ] ; then
	cp l0dable/$L0DABLE.lcd $TARG/files/
fi


if grep -q 'define ENCRYPT_L0DABLE 1' SECRETS ; then

	echo "###"
	echo "### Crypting loadables"
	echo "###"
	
	skey=`./getkey.pl l0dable_sign`
	ekey=`./getkey.pl l0dable_crypt`
	
	if [ -z "$ekey" ] ; then 
	    echo E-Key fail
	    exit 1
	fi
	
	if [ -z "$skey" ] ; then 
	    echo S-Key fail
	    exit 1
	fi
	
	for a in $TARG/files/*.c0d $TARG/files/*.int $TARG/files/*.nik ; do
	    echo Crypting $a
	    ../tools/crypto/xxtea -e -k $ekey $a
	    ../tools/crypto/xxtea -s -k $skey $a
	done

fi

echo "###"
echo "### Done. Yay!"
echo "###"

git checkout SECRETS
git checkout filesystem/ffconf.h
