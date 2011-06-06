#!/bin/bash

# setup
WORK=`pwd`
DATE=$(date +%m%d)

# execution!
cd ..

# check for voodoo/non-voodoo
if [ "$1" != "V" ]; then
	CONFIG="novoodoo"
	cd charge_initramfs
	git checkout froyo
	mv .git ../
else
	CONFIG="voodoo"
	cd charge_initramfs
	git checkout froyo-voodoo
	mv .git ../
fi

# build the kernel
cd $WORK
echo "***** Building : $CONFIG *****"
make clean mrproper > /dev/null 2>&1
rm -f update/*.zip update/kernel_update/zImage

make ARCH=arm charge_defconfig 1>/dev/null 2>"$WORK"/errlog.txt
make -j6 CROSS_COMPILE=/usr/bin/arm-linux-gnueabi- ARCH=arm HOSTCFLAGS="-g -O3" 1>"$WORK"/stdlog.txt 2>>"$WORK"/errlog.txt
if [ $? != 0 ]; then
		echo -e "FAIL!\n"
		cd ..
		mv .git charge_initramfs/
		exit 1
	else
		echo -e "Success!\n"
		rm -f "$WORK"/*log.txt
fi

cp arch/arm/boot/zImage update/kernel_update/zImage
cd update
zip -r -q kernel_update.zip .
mv kernel_update.zip ../"$DATE"_charge_"$CONFIG".zip
cd ../../
mv .git charge_initramfs/
cd $WORK
echo -e "***** Successfully compiled: $CONFIG *****\n"

if [ "$1" == "A" ]; then ./build_kernel.sh V; fi
