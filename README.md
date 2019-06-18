# PhosGB

An experimental Gameboy and Gameboy Color Emulator written in modern C++

<p align="center">
  <img src="examples/Zelda_1.png" hspace="10">
  <img src="examples/Zelda_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/F1-Race_1.png" hspace="10">
  <img src="examples/F1-Race_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/SuperMarioLand_1.png" hspace="10">
  <img src="examples/SuperMarioLand_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/PokemonRed_1.png" hspace="10">
  <img src="examples/PokemonRed_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/Tetris_1.png" hspace="10">
  <img src="examples/Tetris_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/Debugger.png">
</p>

## Features

TODO

## Building

PhosGB currently supports Windows, Linux and macOS. The frontend requires SDL2.

### Linux

Install SDL2 through your package manager, e.g. ```apt install libsdl2-dev```

``` mkdir build && cd build ```

``` cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . --target phos -j 2 ```

### Windows

TODO

### macOS

``` brew install sdl2 cmake ```

``` mkdir build && cd build ```

``` cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . --target phos -j 2 ```

## Accuracy

TODO


