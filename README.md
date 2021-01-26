# 420 colours

Useless app which shows us very colourful maze.

# Usage

Usage is very complicated.
Launch program like any executable file.

If you did this difficult step, then the app should start.

Controls:

R -- restart

S -- stop generation and start colouring

E -- switch colours to some weird raindow and vice versa

Alt+F4 or some other quit signal from window -- you won't guess -> it will close the program

# Compilation

There is some cmake file, but the cpp file is only one, so it is easy

```
mkdir build; cd build; cmake ..; make
```
or
```
g++ -o 420-colours --std=c++17 -lfmt -lGL -lGLEW -lSDL2 main.cpp
```

# Dependencies

* C++
* SDL2
* GLEW
* {fmt}
* glm

# :>

Bon appetit!
