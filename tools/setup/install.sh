#!/bin/sh
cd .dependencies

cd cpp_tools
make PREFIX=../libsimple_support ../libsimple_support/include/make_templates/header_only_lib
make PREFIX=../libsimple_geom ../libsimple_geom/include/make_templates/header_only_lib
make PREFIX=../libsimple_sdlcore ../libsimple_sdlcore/include/make_templates/static_lib
make PREFIX=../libsimple_graphical ../libsimple_graphical/include/make_templates/static_lib
make PREFIX=../libsimple_interactive ../libsimple_interactive/include/make_templates/static_lib
make PREFIX=../libsimple_musical ../libsimple_interactive/include/make_templates/static_lib
make PREFIX=../nanovg ../nanovg/include/make_templates/static_lib
cd ..

cd libsimple_support
make install PREFIX=../../  "$@"
make install PREFIX=../libsimple_sdlcore "$@"
make install PREFIX=../libsimple_graphical "$@"
make install PREFIX=../libsimple_interactive "$@"
make install PREFIX=../libsimple_musical "$@"
cd ..

cd libsimple_geom
make install PREFIX=../../  "$@"
make install PREFIX=../libsimple_sdlcore "$@"
make install PREFIX=../libsimple_graphical "$@"
make install PREFIX=../libsimple_interactive "$@"
make install PREFIX=../libsimple_musical "$@"
cd ..

cd libsimple_sdlcore
make install PREFIX=../../ "$@"
make install PREFIX=../libsimple_graphical "$@"
make install PREFIX=../libsimple_interactive "$@"
make install PREFIX=../libsimple_musical "$@"
cd ..

cd libsimple_graphical
make install PREFIX=../../ "$@"
cd ..

cd libsimple_interactive
make install PREFIX=../../ "$@"
cd ..

cd libsimple_musical
make install PREFIX=../../ "$@"
cd ..

cd nanovg
make install GL=2 PREFIX=../../ "$@"
cd ..

cd ..
