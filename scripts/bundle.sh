#!/bin/bash

set -euo pipefail

docker build -f build.Dockerfile --build-arg HOST_UID=$(id -u) -t devkitvb-build .
docker run --rm -v .:/app -w /app devkitvb-build /app/scripts/do-bundle.sh
./scripts/windows-installer.sh
