#!/bin/bash

set -euo pipefail

TARGET_PATH="$1"

cd tools/picolibc
./bundle.sh
cd ../..
cp -r tools/picolibc/build-bundle "$TARGET_PATH"
