#!/bin/bash

set -e

VERSION=5.5

# Get depot_tools
if [ ! -d "depot_tools" ]; then
  echo "Cloning depot_tools"
  git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

export PATH="`pwd`/depot_tools":"$PATH"

# Get v8 sources
if [ ! -d "v8" ]; then
  echo "Fetching v8"
  fetch v8
fi

echo "Checkout the $VERSION branch"
pushd v8
git checkout remotes/branch-heads/$VERSION
popd
