#!/bin/bash

set -euo pipefail

LLVM_VERSION="2026-10-18"

rm -rf build
mkdir build

# install the native v810-llvm
./scripts/install-v810-llvm.sh "$LLVM_VERSION" linux build/linux

# add tools from it to our path
mkdir build/bin
ln -s $(pwd)/build/linux/llvm-v810/bin/clang build/bin/v810-unknown-vb-clang
ln -s $(pwd)/build/linux/llvm-v810/bin/llvm-ar build/bin/v810-unknown-vb-llvm-ar
ln -s $(pwd)/build/linux/llvm-v810/bin/llvm-mc build/bin/v810-unknown-vb-llvm-mc
ln -s $(pwd)/build/linux/llvm-v810/bin/ld.lld build/bin/v810-unknown-vb-ld.lld
ln -s $(pwd)/build/linux/llvm-v810/bin/llvm-strip build/bin/v810-unknown-vb-llvm-strip
export PATH="$PATH:$(pwd)/build/bin"

# build picolibc
./scripts/install-picolibc.sh build/picolibc

# build vbcc for each target
./scripts/install-vbcc.sh "windows" "build/windows"
./scripts/install-vbcc.sh "darwin" "build/darwin"
./scripts/install-vbcc.sh "linux" "build/linux"

# install the remaining v810-llvms
./scripts/install-v810-llvm.sh "$LLVM_VERSION" windows build/windows
./scripts/install-v810-llvm.sh "$LLVM_VERSION" darwin build/darwin

# and pack that shit
./scripts/pack.sh "windows"
./scripts/pack.sh "darwin"
./scripts/pack.sh "linux"
