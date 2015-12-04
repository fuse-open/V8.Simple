#!/bin/sh
export LDFLAGS=" -L$HOME/v8-lib/lib-osx-32 -stdlib=libstdc++"
export CXXFLAGS=" -I$HOME/v8 -m32 -stdlib=libstdc++"
make
