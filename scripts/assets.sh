#!/bin/bash

SOURCE="src/data"
DESTINATION="content/compiled"

cd "$SOURCE"
rm $DESTINATION/*.h $DESTINATION/*.c
grit *.bmp -ftc -pS -gB8 -gT ff00ff -O palette_song.c
grit spr_arrows.bmp spr_numbers.bmp -ftc -pS -gB8 -gT ff00ff -O palette_selection.c
mv *.h "$DESTINATION"
mv *.c "$DESTINATION"
