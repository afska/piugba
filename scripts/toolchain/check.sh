function try {
  "$@"
  local status=$?
  if [ $status -eq 127 ]; then
      echo "❌  $1" >&2
      exit $?
  fi
}

try gbfs > /dev/null
echo "✔️  gbfs"

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
