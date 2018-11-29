#!/bin/sh
cd .dependencies

cd libsimple_support
make install PREFIX=../../  "$@"
make install PREFIX=../libsimple_sdlcore
make install PREFIX=../libsimple_graphical
make install PREFIX=../libsimple_interactive
cd ..

cd libsimple_geom
make install PREFIX=../../  "$@"
make install PREFIX=../libsimple_sdlcore
make install PREFIX=../libsimple_graphical
make install PREFIX=../libsimple_interactive
cd ..

cd libsimple_sdlcore
make install PREFIX=../../ "$@"
make install PREFIX=../libsimple_graphical
make install PREFIX=../libsimple_interactive
cd ..

cd libsimple_graphical
make install PREFIX=../../ "$@"
cd ..

cd libsimple_interactive
make install PREFIX=../../ "$@"
cd ..

cd nanovg
make install GL=2 PREFIX=../../ "$@"
cd ..

cd ..
