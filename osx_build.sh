#!/bin/sh
export LDFLAGS=" -install_name @rpath/libV8Simple.dylib -Ldeps/libs/osx -lv8_base -lv8_libbase -lv8_libplatform -lv8_libsampler -lv8_nosnapshot"
export CXXFLAGS=" -Oz -Ideps -arch i386 -arch x86_64 -fvisibility=hidden -fvisibility-inlines-hidden -DBUILDING_DLL"
export OBJ_DIR="obj/osx"
make lib/V8Simple.net.dll lib/libV8Simple.dylib
make check
