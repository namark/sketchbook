#!/bin/sh

CXXFLAGS="$CXXFLAGS -s USE_SDL=2" LDFLAGS="$LDFLAGS -s USE_SDL=2" ./tools/setup/install.sh CXX=emcc LOCAL_TEMP=web LOCAL_DIST=web INSTALL_LIB=lib/web "$@"
