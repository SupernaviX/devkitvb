#!/bin/bash

set -euo pipefail

TARGET_NAME="$1" # windows, darwin, linux
TARGET_PATH="$2"

case $TARGET_NAME in
    "linux")
        flags=""
        targetDir="target/release"
        exeExt=""
        ;;
    "darwin")
        flags="--target=aarch64-apple-darwin"
        targetDir="target/aarch64-apple-darwin/release"
        exeExt=""
        ;;
    "windows")
        flags="--target=x86_64-pc-windows-msvc"
        targetDir="target/x86_64-pc-windows-msvc/release"
        exeExt=".exe"
        ;;
    *)
        echo "unknown target $TARGET_NAME"
        exit 1
        ;;
esac

bins=(vbgfx)

cargo build --release $flags
mkdir -p "$TARGET_PATH"
for bin in $bins; do
    cp "$targetDir/$bin$exeExt" "$TARGET_PATH/"
done
