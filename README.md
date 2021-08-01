# memory basics

g++ main.cpp

### Hosts a few home-grown utilities, including:

- Basic memory management structures: General purpose, pool and stack allocators.

- An array wrapper to C-style arrays, holding the length parameter and adding support for Add(), Insert() and Remove().

- A wrapper for random number generation found on the web (see random.h for details).

These are intended as light-weight alternatives: Compact, understandable and modifiable.

The CMakeLists.txt file serves as the vscode project file, and as such it simply builds main.cpp.
