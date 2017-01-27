#!/usr/bin/env bash

set -euo pipefail

brew update
brew install qbs tree

clang --version
clang++ --version

exit 0
