
#!/bin/sh
cd .dependencies

cd libsimple_support
git pull -r
cd ..

cd libsimple_geom
git pull -r
cd ..

cd libsimple_sdlcore
git pull -r
cd ..

cd libsimple_graphical
git pull -r
cd ..

cd libsimple_interactive
git pull -r
cd ..

cd nanovg
git pull -r
cd ..

cd ..

./tools/setup/install.sh "$@"

