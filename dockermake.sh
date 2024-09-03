#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <make-target> [make-arguments...]"
    exit 1
fi

MAKE_TARGET=$1
shift
MAKE_ARGS="$@"

docker run --rm \
  -v "$(pwd)":/opt/piugba \
  -e PWD=/opt/piugba \
  afska/piugba-dev \
  make "$MAKE_TARGET" $MAKE_ARGS
