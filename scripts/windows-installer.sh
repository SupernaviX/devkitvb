#!/bin/bash

set -euo pipefail
mkdir -p bundle
mkdir -p Output

OLDOWNER=$(ls -ld bundle | awk '{print $3 ":" $4}')
NEWUID=$(docker run --rm --entrypoint bash amake/innosetup -c 'id -u')
NEWGID=$(docker run --rm --entrypoint bash amake/innosetup -c 'id -g')
NEWOWNER="${NEWUID}:${NEWGID}"

sudo chown -R "$NEWOWNER" bundle
sudo chown -R "$NEWOWNER" Output
docker run --rm -v .:/work amake/innosetup installer.iss
sudo chown -R "$OLDOWNER" bundle
sudo chown -R "$OLDOWNER" Output

mv Output/* bundle
rmdir Output
