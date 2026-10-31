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
- [ ] Render my first 3D triangle
- [ ] Make something playable!

## Build

With [Simulant Tools](https://gitlab.com/simulant/simulant-tools) and the Dreamcast Docker SDK configured:

```bash
simulant update -b next
simulant build dreamcast
simulant package dreamcast
```

The Dreamcast image is generated in `packages/dreamcast-sh4-gcc/`.

*More to come!*
