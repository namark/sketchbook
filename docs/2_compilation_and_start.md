# Compilation and start

1. Initially run `./tools/setup/init.sh` (optionally providing make parameters), to fetch and install dependencies.
```bash
./tools/setup/init.sh CXX=g++-7
```

2. To compile the project itself, use `make`
```bash
make CXX=g++-7
```

3. After compilation executable files will be created in the `out` folder.
```bash
./out/name_of_the_sketch
```
