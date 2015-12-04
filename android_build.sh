#!/bin/sh
NDK_DIR="/usr/local/share/uno/SDKs/AndroidNDK"
PREBUILT_DIR="$NDK_DIR/toolchains/arm-linux-androideabi-4.8/prebuilt"
SYSROOT_DIR="$NDK_DIR/platforms/android-9/arch-arm"
STLPORT_DIR="$NDK_DIR/sources/cxx-stl/stlport"
export CXX=`find "$PREBUILT_DIR" -name arm-linux-androideabi-g++`
COMMON_FLAGS="--sysroot=$SYSROOT_DIR -isystem $STLPORT_DIR/stlport -isystem $SYSROOT_DIR/usr/include"
export LDFLAGS=" -L$HOME/v8-lib/lib-android $COMMON_FLAGS -fdata-sections -ffunction-sections -Wl,--whole-archive $STLPORT_DIR/libs/armeabi-v7a/libstlport_static.a -Wl,--no-whole-archive -Wl,--gc-sections -Wl,--strip-all"
export CXXFLAGS=" -I$HOME/v8 $COMMON_FLAGS"
make lib/libV8Simple.so
