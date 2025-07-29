#!/bin/bash

set -euo pipefail

TAG=$(git describe --abbrev=0 --tags)
REVISION=$(git rev-parse --short HEAD)

echo "$TAG-$REVISION"
