# Basic system setup

Required software:
1. Git (for scripts in tools/setup/).
2. C++ 17 compliant compiler.
3. SDL 2.0.0 library or newer

### Ubuntu / Debian

1. Install Git
```bash
sudo apt install git
```

2. Install sufficiently recent compiler
```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-7 -y
```

3. Install SDL 2 library
```bash
sudo apt install libsdl2-2.0
sudo apt install libsdl2-dev
```

Steps tested on: Ubuntu 14.04, 16.04, 18.04

# Troubleshooting

The sources also contain troubleshooting steps.
In case of any failure make sure to check them first.

# Sources

1. Adding repository "universe" in Apt
http://askubuntu.com/a/227788

2. Installation of SDL 2 on Ubuntu
https://askubuntu.com/questions/707001/ubuntu-15-10-installing-sdl2

3. Installation of G++-7 compiler on Ubuntu
https://gist.github.com/jlblancoc/99521194aba975286c80f93e47966dc5
