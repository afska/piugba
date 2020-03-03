#!/bin/bash

SOURCE="src/data"
DESTINATION="content/compiled"

cd "$SOURCE"
grit *.bmp -ftc -pS -gB8 -gT ff00ff -O shared_palette.c
mv *.h "$DESTINATION"
mv *.c "$DESTINATION"
