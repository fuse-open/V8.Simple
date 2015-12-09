#!/bin/sh
export LDFLAGS=" -Ldeps/libs/osx -stdlib=libc++ -lv8_base -lv8_libbase -lv8_libplatform -lv8_nosnapshot"
export CXXFLAGS=" -Ideps -m32 -stdlib=libc++"
export OBJ_DIR="obj/osx"
make
