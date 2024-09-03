#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <make-target> [make-arguments...]"
    exit 1
fi

MAKE_TARGET=$1
shift
MAKE_ARGS="$@"

wsl -e ./dockermake.sh "$MAKE_TARGET" $MAKE_ARGS
