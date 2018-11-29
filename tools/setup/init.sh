#!/bin/sh

mkdir .dependencies
rm -rf .dependencies/*
cd .dependencies
git clone https://notabug.org/namark/libsimple_support
git clone https://notabug.org/namark/libsimple_geom
git clone https://notabug.org/namark/libsimple_sdlcore
git clone https://notabug.org/namark/libsimple_graphical
git clone https://notabug.org/namark/libsimple_interactive
git clone https://notabug.org/namark/nanovg
cd ..
./tools/setup/install.sh "$@"
