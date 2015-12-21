#!/bin/sh
export LDFLAGS=" -Ldeps/libs/osx -stdlib=libstdc++ -lv8_base -lv8_libbase -lv8_libplatform -lv8_nosnapshot"
export CXXFLAGS=" -Ideps -arch i386 -arch x86_64 -stdlib=libstdc++"
export OBJ_DIR="obj/osx"
make lib/V8Simple.net.dll lib/libV8Simple.dylib
make check
