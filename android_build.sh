#!/bin/sh
NDK_DIR="/Users/olle/build-v8/ndk12b/android-ndk-r12b"
# NDK_DIR="/usr/local/share/uno/SDKs/AndroidNDK"
PREBUILT_DIR="$NDK_DIR/toolchains/arm-linux-androideabi-4.9/prebuilt"
SYSROOT_DIR="$NDK_DIR/platforms/android-16/arch-arm"
STDLIB_DIR="$NDK_DIR/sources/cxx-stl/llvm-libc++"
STDLIB_INCLUDE_DIR="$STDLIB_DIR/libcxx/include"
STDLIB_LIB_DIR="$STDLIB_DIR/libs/armeabi-v7a"
STDLIB="c++_static"
STDLIBABI="c++abi"
SUPPORT_INCLUDE_DIR="$NDK_DIR/sources/android/support/include"
export OBJ_DIR="obj/android"
export CXX=`find "$PREBUILT_DIR" -name arm-linux-androideabi-g++`
COMMON_FLAGS="--sysroot=$SYSROOT_DIR -isystem $STDLIB_INCLUDE_DIR -isystem $SYSROOT_DIR/usr/include -I$SUPPORT_INCLUDE_DIR"
export LDFLAGS=" -L$SYSROOT_DIR/usr/lib -Ldeps/libs/android $COMMON_FLAGS -Wl,--whole-archive -lv8_base -lv8_libbase -lv8_libplatform -lv8_libsampler -lv8_nosnapshot -L$STDLIB_LIB_DIR -landroid_support -lunwind -llog -l$STDLIB -l$STDLIBABI -Wl,--no-whole-archive -Wl,--gc-sections -Wl,-soname,libV8Simple.so -Wl,--strip-all"
# export LDFLAGS=" -Ldeps/libs/android $COMMON_FLAGS -fdata-sections -ffunction-sections -Wl,--whole-archive -lv8_base -lv8_libbase -lv8_libplatform -lv8_libsampler -lv8_nosnapshot -L$STDLIB_LIB_DIR -landroid_support -lunwind -l$STDLIB -Wl,--no-whole-archive -Wl,--gc-sections -Wl,-soname,libV8Simple.so -Wl,--strip-all"
export CXXFLAGS=" -shared -Os -Ideps $COMMON_FLAGS -fvisibility=hidden -fvisibility-inlines-hidden -DBUILDING_DLL -fdata-sections -ffunction-sections"
make lib/libV8Simple.so
