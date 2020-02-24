#!/bin/bash

FILE_INPUT="pumpitup-advance.gba"
FILE_OUTPUT="pumpitup-advance.out.gba"
DATA="gsmsongs.gbfs"
REQUIRED_SIZE_KB=768 # needs to be multiple of 256

ROM_SIZE=$(wc -c < $FILE_INPUT)
PAD_NEEDED=$((($REQUIRED_SIZE_KB * 1024) - $ROM_SIZE))
dd if=/dev/zero bs=1 count=$PAD_NEEDED >> $FILE_INPUT
cat $FILE_INPUT $DATA > $FILE_OUTPUT
