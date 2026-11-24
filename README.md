<p align="left">
  <img src="assets/readme/dc-swirl.png" alt="Dreamcast swirl" width="72">
</p>

# Dreamcast Homebrew

[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Dreamcast](https://img.shields.io/badge/SEGA-Dreamcast-ed8432)](https://en.wikipedia.org/wiki/Dreamcast)
[![Simulant](https://img.shields.io/badge/Engine-Simulant-7952b3)](https://gitlab.com/simulant/simulant)


My first steps into **Dreamcast homebrew development**! A little 3D game project written in C++, using [Simulant](https://gitlab.com/simulant/simulant) and [KallistiOS](https://github.com/KallistiOS/KallistiOS).

## The prototype

Still very early in development :p

A small courtyard to explore in third person, with a block character, obstacles,
an arch and a stepped platform.

There is now a short quest with 2 NPCs: find a broken mechanical part on
the platform, get it repaired and bring it back. The character has movement
collisions, and the following camera moves closer when the scenery gets in
the way. Picking up the part also checks the height of the character's feet.

Audio engine set up with a background song and short sounds for starting conversations,
collecting the part and using the pause menu. The menu lets you resume, adjust
music volume or restart the quest. Music keeps playing during the pause;
movement, camera controls and dialogue progression stop. Volume starts at 50%
and stays in memory for the session, without a VMU save.

## Controls

| Action | Dreamcast controller |
| --- | --- |
| Walk relative to the camera | Analog stick |
| Orbit the camera left / right | L / R triggers or D-pad left / right |
| Raise / lower the camera angle | D-pad up / down |
| Talk / collect / advance dialogue | A |
| Open / close the pause menu | Start |
| Restart the quest and return to the starting position | Pause menu: Recommencer la quete |

In the pause menu, use D-pad up / down to select, left / right to change volume
in 10% steps (0–100%), A to confirm, and B or Start to resume.

## Build

With [Simulant Tools](https://gitlab.com/simulant/simulant-tools) and the Dreamcast Docker SDK configured:

```bash
simulant build dreamcast
simulant package dreamcast
```

The image is generated at `packages/dreamcast-sh4-gcc/dreamcast-next.cdi`.

## Checks

Native checks for movement, camera collisions, interactions, the pause menu and
music decoding can run without the Dreamcast SDK:

```bash
for name in walking camera_collision interaction pause_menu music; do
    g++ -std=c++17 -O2 -Wall -Wextra "tests/test_${name}.cpp" -o "/tmp/dreamcast-test-${name}" && "/tmp/dreamcast-test-${name}"
done
python3 tests/test_sfx_assets.py
```


Music: Telohub by Panock · SFX: Cozy UI Sounds by Spaceyyy and Cozy Story SFX by Bramble & Byte · Dreamcast help: Mouse's Mistake.
