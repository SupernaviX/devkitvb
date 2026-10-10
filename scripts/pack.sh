#!/bin/bash

set -euo pipefail

TARGET_NAME="$1" # windows, darwin, linux

case $TARGET_NAME in
    "linux" | "darwin")
        exeExt=""
        ;;
    "windows")
        exeExt=".exe"
        ;;
    *)
        echo "unknown target $TARGET_NAME"
        exit 1
        ;;
esac

rm -rf "bundle/$TARGET_NAME"
mkdir -p "bundle/$TARGET_NAME"

mkdir "bundle/$TARGET_NAME/bin"
cp "build/$TARGET_NAME/vbcc$exeExt" "bundle/$TARGET_NAME/bin"
cp -r "build/$TARGET_NAME/llvm-v810" "bundle/$TARGET_NAME/clang"
mkdir "bundle/$TARGET_NAME/lib"
mkdir "bundle/$TARGET_NAME/lib/vb"
cp -r build/picolibc/include "bundle/$TARGET_NAME/lib/vb/include"
cp build/picolibc/lib/* "bundle/$TARGET_NAME/lib/vb"
cp tools/libvb/* "bundle/$TARGET_NAME/lib/vb"
mv bundle/$TARGET_NAME/lib/vb/*.h bundle/$TARGET_NAME/lib/vb/include

cd bundle
zip -r "devkitvb-$TARGET_NAME.zip" "$TARGET_NAME"
