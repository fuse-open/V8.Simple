#!/bin/sh
export LDFLAGS=" -Ldeps/libs/osx -stdlib=libstdc++ -lv8_base -lv8_libbase -lv8_libplatform -lv8_nosnapshot"
export CXXFLAGS=" -Ideps -m32 -stdlib=libstdc++"
export OBJ_DIR="obj/osx"
make
