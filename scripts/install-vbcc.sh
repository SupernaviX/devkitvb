#!/bin/bash

set -euo pipefail

TARGET_NAME="$1" # windows, darwin, linux
TARGET_PATH="$2"

case $TARGET_NAME in
    "linux")
        cc=clang
        exeExt=""
        cflags="-Wall -Wextra -Werror -O2 -flto"
        ;;
    "darwin")
        cc=oa64-clang
        exeExt=""
        cflags="--target=aarch64-apple-darwin -Wall -Wextra -Werror -O2 -flto"
        ;;
    "windows")
        cc=clang
        exeExt=".exe"
        cflags="--target=x86_64-pc-windows-msvc -O2 -flto -fuse-ld=lld-link -I/xwin/crt/include -I/xwin/sdk/include/ucrt -I/xwin/sdk/include/um -I/xwin/sdk/include/shared -L/xwin/crt/lib/x86_64 -L/xwin/sdk/lib/um/x86_64 -L/xwin/sdk/lib/ucrt/x86_64"
        ;;
    *)
        echo "unknown target $TARGET_NAME"
        exit 1
        ;;
esac

cd tools/vbcc
make vbcc CC=$cc CFLAGS="$cflags"
cd ../..
mkdir -p "$TARGET_PATH"
cp "tools/vbcc/vbcc" "$TARGET_PATH/vbcc$exeExt"
