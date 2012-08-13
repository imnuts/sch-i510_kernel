#!/bin/bash

# setup
WORK=`pwd`
DATE=$(date +%m%d)

# Move .git so we don't have a commit hash in the kernel version
mv .git ../.git

# build the kernel
echo "***** Building *****"
make mrproper > /dev/null 2>&1
rm -f update/*.zip update/kernel_update/zImage

make ARCH=arm charge_defconfig 1>/dev/null 2>"$WORK"/errlog.txt
make -j8 HOSTCFLAGS="-g -O3" 1>"$WORK"/stdlog.txt 2>>"$WORK"/errlog.txt
if [ $? != 0 ]; then
		echo -e "EPIC FAIL!\n\n"
		cd ..
		mv .git "$WORK"/.git
		exit 1
	else
		echo -e "Great Success!\n"
		rm -f "$WORK"/*log.txt
fi

# Build a recovery odin file
cp arch/arm/boot/zImage recovery.bin
tar -c recovery.bin > "$DATE"_charge_recovery.tar
md5sum -t "$DATE"_charge_recovery.tar >> "$DATE"_charge_recovery.tar
mv "$DATE"_charge_recovery.tar "$DATE"_charge_recovery.tar.md5
rm recovery.bin

# Make the CWM Zip
cp arch/arm/boot/zImage update/kernel_update/zImage
cd update
zip -r -q kernel_update.zip .
mv kernel_update.zip ../"$DATE"_charge.zip

# Finish up
cd ../../
mv .git $WORK/.git
cd $WORK
echo -e "***** Finish *****\n"
