# Wii Horizon — first 3D prototype

A small Nintendo Wii homebrew driving prototype using devkitPPC and libogc.

## Controls
- D-pad Left / Right: steer
- A: accelerate
- B: brake
- HOME: exit

## Build
The GitHub Actions workflow uses the devkitPro devkitPPC container and runs `make`.
The expected output is `horizon.dol`.

## Notes
This first attempt uses colored GX geometry only (no external models or textures).
It is a prototype, not a full Forza Horizon game. Test first in an emulator if available,
then on a Wii with a working Homebrew Channel.
