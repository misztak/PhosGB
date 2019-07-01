# PhosGB

An experimental Gameboy and Gameboy Color Emulator written in modern C++

<p align="center">
  <img src="examples/PokemonYellow_1.png" hspace="10">
  <img src="examples/PokemonYellow_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/DKC_1.png" hspace="10">
  <img src="examples/DKC_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/WarioLand3_1.png" hspace="10">
  <img src="examples/WarioLand3_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/Zelda_1.png" hspace="10">
  <img src="examples/Zelda_2.5.png" hspace="10">
</p>
<p align="center">
  <img src="examples/F1-Race_1.png" hspace="10">
  <img src="examples/F1-Race_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/Tetris_1.png" hspace="10">
  <img src="examples/Tetris_2.png" hspace="10">
</p>
<p align="center">
  <img src="examples/Debugger.png">
</p>

## Features

* WIP Gameboy Color support

* Support for MBC1, MBC2, MBC3 and MBC5 cartridges

* Full audio emulation

* Save States & Fast Forward

* Screenshots

* Saves SRAM to file

* Debugger with Memory Editor, Tileset & Background Viewer, etc.

* Custom palette for DMG mode

### Input

<img src="examples/Controls.png" hspace="15">

| Key           | Function      |
| ------------- |:-------------:|
| ```G```       | Toggle Debug Mode |
|```H```|Pause|
| ```F```      | Fast Forward      |
| ```F5``` | Save State      |
| ```F6```| Load State |
|```M```| Change Frame-Timer Mode |

**Right-Click** opens the main menu in Normal Mode.


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


