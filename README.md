<p align="left">
  <img src="assets/readme/dc-swirl.png" alt="Dreamcast swirl" width="72">
</p>

# Dreamcast Homebrew

[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Dreamcast](https://img.shields.io/badge/SEGA-Dreamcast-ed8432)](https://en.wikipedia.org/wiki/Dreamcast)
[![Simulant](https://img.shields.io/badge/Engine-Simulant-7952b3)](https://gitlab.com/simulant/simulant)


My first steps into **Dreamcast homebrew development**! A little 3D game project written in C++, using [Simulant](https://gitlab.com/simulant/simulant) and [KallistiOS](https://github.com/KallistiOS/KallistiOS).

## Current state

Still very early in development :p

- [x] Set up the Dreamcast toolchain
- [x] Build a `.cdi` and run it in Flycast
- [x] Render a custom background
- [x] Render my first 3D triangle
- [x] Walk around a small 3D scene in third person
- [ ] Add a game objective

## Exploration prototype

A small courtyard with a visible block character, a checkerboard floor, colored
blocks, an arch and our original triangle on a pedestal. A directional light
helps distinguish the faces of the geometry. The blue background and Simulant
splash are preserved.

| Action | Dreamcast controller | Keyboard exposed to the engine |
| --- | --- | --- |
| Walk relative to the camera | Analog stick | WASD or ZQSD |
| Orbit the camera left / right | X / B or D-pad left / right | Left / right arrows |
| Raise / lower the camera angle | Y / A or D-pad up / down | Up / down arrows |
| Return to the starting position | Start | R |

In Flycast, configure the host keyboard or gamepad as a Dreamcast controller;
the game reads the emulated controller. The keyboard bindings above also support
a keyboard supplied directly to Simulant.

The player stays on the flat ground. Static boxes stop movement and allow sliding
along their sides; the camera moves closer when a block obstructs its view.
Movement speed is multiplied by frame time, and diagonal input is capped to the
same maximum speed as straight input.

`sources/scenes/game.cpp` creates the world, reads input, moves the character and
updates the camera. `sources/scenes/walking.h` contains the independent XZ
collision logic, and `sources/scenes/camera_collision.h` keeps the camera outside
the static geometry. The character is a `Stage` grouping its body, head and nose;
the perspective camera follows it while looking at a point above its feet.

## Build

With [Simulant Tools](https://gitlab.com/simulant/simulant-tools) and the Dreamcast Docker SDK configured:

```bash
simulant update -b next
simulant build dreamcast
simulant package dreamcast
```

The Dreamcast image is generated in `packages/dreamcast-sh4-gcc/`.

The player and camera collision checks can be tested without the Dreamcast SDK:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_walking.cpp -o /tmp/dreamcast-next-walking-test
/tmp/dreamcast-next-walking-test
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_camera_collision.cpp -o /tmp/dreamcast-next-camera-test
/tmp/dreamcast-next-camera-test
```
