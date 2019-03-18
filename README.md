# visualizer
superbly simple sdl + fftw audio visualizer built for pulse

## features missing
* better aesthetics

![main and only window](/images/window.png)

## configuration
for configuration options, see [src/main.cpp](/src/main.cpp)

## building and dependencies
this program requires:
* [SDL2](https://www.libsdl.org/)
* [fftw3](http://fftw.org/)
* [pulseaudio-simple](https://www.freedesktop.org/wiki/Software/PulseAudio/)

compile-time dependencies:
* [cmake](https://cmake.org/)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)

build process:
```bash
mkdir build
cd build
cmake ..
make
./sdl-fft-visualizer # done
```
