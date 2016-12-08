#!/bin/bash

set -e

source fetch.sh

echo "Syncing dependencies"
# gclient sync

echo "Building"
pushd v8
gn gen out.gn/Android.release -vv --fail-on-unused-args --args='android_ndk_root="/usr/local/share/uno/SDKs/AndroidNDK" is_debug=false is_official_build=true target_os="android" symbol_level=1 v8_enable_i18n_support=false'
ninja -C out.gn/Android.release
for f in {v8_base,v8_external_snapshot,v8_libbase,v8_libplatform,v8_nosnapshot,v8_snapshot}
do
	echo "making lib for $f"
	./third_party/android_tools/ndk/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ar rcsD "out.gn/Android.release/lib${f}.a" "out.gn/Android.release/obj/$f/"*.o
done
popd
