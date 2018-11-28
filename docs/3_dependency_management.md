# Dependency management

### Clone and install dependencies
```bash
./tools/setup/init.sh CXX=g++-7
```
### Update and reinstall dependencies
```bash
./tools/setup/update.sh CXX=g++-7
```
### Install dependencies
```bash
./tools/setup/install.sh CXX=g++-7
```
Note: This script is used internally by `init.sh` and `update.sh`, so you don't usually need it, but it can be handy in some situations. For example, if compilation fails because of wrong make parameters, you can skip the cloning/updating phase, and jump to installation directly.


# Troubleshooting
1. Make sure you are using C++ 17 compliant compiler. You might need to explicitly specify it, as shown in the examples.
2. Make sure to run the scripts from the project root.
