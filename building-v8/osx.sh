#!/bin/bash

set -e

source fetch.sh

echo "Building"
pushd v8
make -j4 i18nsupport=off OUTDIR=../out ia32.release
make -j4 i18nsupport=off OUTDIR=../out x64.release
popd

echo "Creating universal library"
mkdir -p out/universal.release
for f in {libv8_base.a,libv8_external_snapshot.a,libv8_libbase.a,libv8_libplatform.a,libv8_libsampler.a,libv8_nosnapshot.a}; do
  lipo -create out/ia32.release/$f out/x64.release/$f -output out/universal.release/$f
done
