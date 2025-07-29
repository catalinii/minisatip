#!/bin/bash

set -euo pipefail

TAG=$(git describe --abbrev=0 --tags)
REVISION=$(git rev-parse --short HEAD)
PR=${PR:-}

if [ -n "$PR" ]; then
  echo "$TAG+PR$PR~$REVISION"
else
  echo "$TAG~$REVISION"
fi
