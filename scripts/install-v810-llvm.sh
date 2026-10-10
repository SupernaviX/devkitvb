#!/bin/bash

set -euo pipefail
IFS=$'\n\t'

REPO_URL="https://github.com/SupernaviX/v810-llvm"
BUILD_DATE="$1"
TARGET_NAME="$2" # windows, darwin, linux
TARGET_PATH="$3"

case $TARGET_NAME in
    "linux" | "darwin")
        bundleExt=tar.xz
        ;;
    "windows")
        bundleExt=7z
        ;;
    *)
        echo "unknown target $TARGET_NAME"
        exit 1
        ;;
esac

mkdir -p "$TARGET_PATH"
cd "$TARGET_PATH"

filename="llvm-v810-$TARGET_NAME-main.$bundleExt"
curl -o "llvm-v810.$bundleExt" -L "$REPO_URL/releases/download/llvm-v810-$BUILD_DATE/$filename"
if [ "$bundleExt" == "7z" ]; then
    7z x "llvm-v810.$bundleExt"
else
    tar -xvf "llvm-v810.$bundleExt"
fi
