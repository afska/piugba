#!/bin/bash

try() {
  "$@"
  status=$?
  if [ $status -eq 127 ]; then
      echo "❌  $1" >&2
      exit $status
  fi
}

if [ -n "$NVM_DIR" ]; then
  source $NVM_DIR/nvm.sh
fi

try gbfs > /dev/null
try pngfix > /dev/null
echo ""
echo "✔️  gbfs"
echo "✔️  pngfix"

try rm --version > /dev/null
echo "✔️  rm"

try dd --version > /dev/null
echo "✔️  dd"

try md5sum --version > /dev/null
echo "✔️  md5sum"

try cut --version > /dev/null
echo "✔️  cut"

try ffmpeg -version > /dev/null
echo "✔️  ffmpeg"

try ffplay -version > /dev/null
echo "✔️  ffplay"

try magick -version > /dev/null
echo "✔️  magick"

try grit > /dev/null
echo "✔️  grit"

try make -version > /dev/null
echo "✔️  make"

try node -v > /dev/null
echo "✔️  node"

echo ""
echo "✔️  all tools are installed!"
