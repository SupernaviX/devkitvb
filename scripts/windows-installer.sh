#!/bin/bash

set -euo pipefail

docker run -it --rm -v .:/app -w /app amake/innosetup installer.iss
mkdir -p bundle
mv Output/* bundle
rmdir Output
