#!/bin/bash

# setup
WORK=`pwd`
DATE=$(date +%m%d)

# execution!
cd ..

# check for device we're building for
if [ "$1" == "strat" ]; then
	DEVICE="stratosphere"
	cd "$DEVICE"_initramfs
	git checkout gingerbread
	mv .git ../
else
	DEVICE="charge"
	cd "$DEVICE"_initramfs
	git checkout gingerbread
	mv .git ../
fi

# build the kernel
cd $WORK
echo "***** Building for $DEVICE *****"
make mrproper > /dev/null 2>&1
rm -f update/*.zip update/kernel_update/zImage

if [ $DEVICE == stratosphere ]; then
make ARCH=arm EXTRAVERSION=.7 "$DEVICE"_defconfig 1>/dev/null 2>"$WORK"/errlog.txt
make -j3 EXTRAVERSION=.7 HOSTCFLAGS="-g -O3" 1>"$WORK"/stdlog.txt 2>>"$WORK"/errlog.txt
else
make ARCH=arm "$DEVICE"_defconfig 1>/dev/null 2>"$WORK"/errlog.txt
make -j3 HOSTCFLAGS="-g -O3" 1>"$WORK"/stdlog.txt 2>>"$WORK"/errlog.txt
fi
if [ $? != 0 ]; then
		echo -e "FAIL!\n\n"
		cd ..
		mv .git "$DEVICE"_initramfs/
		exit 1
	else
		echo -e "Success!\n"
		rm -f "$WORK"/*log.txt
fi

# Build a recovery odin file
cp arch/arm/boot/zImage recovery.bin
tar -c recovery.bin > "$DATE"_"$DEVICE"_recovery.tar
md5sum -t "$DATE"_"$DEVICE"_recovery.tar >> "$DATE"_"$DEVICE"_recovery.tar
mv "$DATE"_"$DEVICE"_recovery.tar "$DATE"_"$DEVICE"_recovery.tar.md5
rm recovery.bin

# Make the CWM Zip
cp arch/arm/boot/zImage update/kernel_update/zImage
cd update
zip -r -q kernel_update.zip .
mv kernel_update.zip ../"$DATE"_"$DEVICE".zip

# Finish up
cd ../../
mv .git "$DEVICE"_initramfs/
cd $WORK
echo -e "***** Successfully compiled for $DEVICE *****\n"

if [ "$1" == "A" ]; then ./build_kernel.sh strat; fi
