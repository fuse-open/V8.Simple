#!/bin/sh
export LDFLAGS=" -L$HOME/v8-lib/lib-osx-32 -stdlib=libstdc++ -lv8_base -lv8_libbase -lv8_libplatform -lv8_nosnapshot"
export CXXFLAGS=" -I$HOME/v8 -m32 -stdlib=libstdc++"
make
