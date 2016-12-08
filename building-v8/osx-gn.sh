#!/bin/bash

# This doesn't quite work because apparently gn has no support for building 32
# bit on OSX. It will probably work in the future.
#
# For now we can use osx.sh

set -e

source fetch.sh

echo "Syncing dependencies"
# gclient sync

echo "Building"
pushd v8
for arch in {ia32,x64}
do
	gn gen out.gn/$arch.release -vv --fail-on-unused-args --args="v8_current_cpu=\"$arch\" is_debug=false is_official_build=true symbol_level=1 v8_enable_i18n_support=false"
	ninja -C out.gn/$arch.release
	for f in {v8_base,v8_external_snapshot,v8_libbase,v8_libplatform,v8_nosnapshot,v8_snapshot}
	do
		echo "making lib for $f"
		ar -r -c -s "out.gn/$arch.release/lib${f}.a" "out.gn/$arch.release/obj/$f/"*.o
	done
done
mkdir -p out.gn/universal.release
for f in {v8_base,v8_external_snapshot,v8_libbase,v8_libplatform,v8_nosnapshot,v8_snapshot}
do
	lipo -create "out.gn/ia32.release/lib${f}.a" "out.gn/x64.release/lib${f}.a" -output "out.gn/universal.release/lib${f}.a"
done
	
popd

