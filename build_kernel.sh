#!/bin/bash

# setup
WORK=`pwd`
DATE=$(date +%m%d)

# execution!
cd ..
mv $WORK/.git .git-kernel
cd charge_initramfs
git checkout gingerbread
mv .git ../.git-initramfs

# build the kernel
cd $WORK
echo "***** Building *****"
make mrproper > /dev/null 2>&1
rm -f update/*.zip update/kernel_update/zImage *.zip

make ARCH=arm charge_defconfig 1>/dev/null 2>"$WORK"/errlog.txt
make -j8 HOSTCFLAGS="-g -O3" 1>"$WORK"/stdlog.txt 2>>"$WORK"/errlog.txt
if [ $? != 0 ]; then
		echo -e "EPIC FAIL!\n\n"
		cd ..
		mv .git-initramfs charge_initramfs/.git
		mv .git-kernel $WORK/.git
		exit 1
	else
		echo -e "GREAT SUCCESS!\n"
		rm -f "$WORK"/*log.txt
fi

# Make the CWM Zip
cp arch/arm/boot/zImage update/kernel_update/zImage
cd update
zip -9 -r -q kernel_update.zip .
mv kernel_update.zip ../"$DATE"_charge.zip

# Finish up
cd ../../
mv .git-initramfs charge_initramfs/.git
mv .git-kernel $WORK/.git
cd $WORK
echo -e "***** Done Building *****\n"
