# baselayer

Written in C-style C++:

- Memory allocation & List
- Vector & matrix 3D math
- Software 2D/3D Renderer and IM-UI library
- Random number generator
- Length-based string utilities
- Types, functions, macros
- Performance profiler
- Algorithms: Octree, Index sets, Tokenizer
- Utils: file I/O, cmd-line args

## dependencies (UI/graphics with OpenGL)

On Ubuntu and similar platforms:

<pre>
sudo apt install libglew-dev
sudo apt install libglfw3-dev
</pre>

## test

First install dependencies, then the following:

<pre>
git clone https://github.com/climbcat/baselayer/
cd baselayer/graphics/build
cmake ..
make
./graphics
</pre>
