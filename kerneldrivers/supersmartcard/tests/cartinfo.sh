#!/bin/sh

if [ ! -c /dev/misc/ssc/flash/boot ]; then
	echo "No ssc device found..."
	exit 1
fi

tmpfile=/tmp/ssctmp_$$

dd if=/dev/misc/ssc/flash/boot bs=324 count=1 of=$tmpfile 2>/dev/null

cardname=$(dd if=$tmpfile bs=1 skip=308 count=16 2>/dev/null)

if [ "$cardname" = "GB SMART CARD" ]; then
	echo "Smart Card:"
	echo "Card 1: $(dd if=$tmpfile bs=1 skip=128 count=16 2>/dev/null)"
	echo "Card 2: $(dd if=$tmpfile bs=1 skip=160 count=16 2>/dev/null)"
	echo "Card 3: $(dd if=$tmpfile bs=1 skip=192 count=16 2>/dev/null)"
	echo "Card 4: $(dd if=$tmpfile bs=1 skip=224 count=16 2>/dev/null)"
else
	echo "Card: $cartname"
fi

rm $tmpfile
