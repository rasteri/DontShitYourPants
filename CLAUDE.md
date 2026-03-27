# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A retro DOS text adventure game written in C, targeting real-mode DOS/CGA graphics. Compiles with OpenWatcom and runs in DOSBox-X. The game has a countdown timer — the player must navigate to a toilet before time runs out.

## Build

Requires OpenWatcom (`wcc`, `wlink`, `wasm`) and DOSBox-X on the path.

```bash
make          # Build dontshit.exe, create floppy image, launch DOSBox-X
make clean    # Remove build artifacts
```

Build outputs object files to `./build/`, final exe to root, then packages everything into a floppy image via `bfi.exe` and launches via `dosbox.conf`.

For the graphics subtest:
```bash
make -f gfxtest.mk    # in gfxtest/
```

## Compiler flags

`wcc -i="C:\WATCOM/h" -w4 -e25 -zq -od -d2 -bt=dos -ml` — large memory model, real-mode DOS, debug symbols. No optimization (`-od`).

## Architecture

### Data-driven state machine

Game content lives in text files loaded at runtime:
- `verbs.txt` — command synonyms (e.g. "shit", "take a shit" → same verb ID)
- `strings.txt` — all display strings, indexed by ID
- `actions.txt` — state × verb → action mappings (the full game logic table)
- `states.txt` — state name reference

Binary graphics are LZ4-compressed `.bin` files (numbered 1–46).

### Control flow

```
main.c (game loop + keyboard input + countdown timer)
  └─ GameLogic_TextInput()  [gamelogic.c]
       ├─ FindVerb()         — parse input against synonym table
       ├─ state table lookup [states.c] — ACTIONS_[STATE] arrays
       └─ action executor    — ACTION_TEXTOUTPUT, ACTION_GOTOSTATE,
                               ACTION_GIVEAWARD, ACTION_PLAYSOUND,
                               ACTION_DISPLAYGFX, etc. (15 types)
```

### Key files

| File | Role |
|------|------|
| `main.c` | Entry point, main loop, timer |
| `gamelogic.c` / `gamelogic.h` | Action executor, verb lookup, awards, shared types |
| `states.c` | Full state×verb→action table (~50 states) |
| `gfx.c` | CGA split-screen, LZ4 decompression, text rendering |
| `tunes.c` | PC speaker synthesis via timer interrupt |
| `load.c` | Parses `strings.txt` and `verbs.txt` at startup |
| `raster.asm` | x86 assembly CGA raster interrupt for split-screen |

### Graphics

CGA 160×100 mode with a split screen: graphics above, text input below. The split is managed by a raster interrupt in `raster.asm` that switches CGA modes mid-frame. Images are LZ4-decompressed at display time. Source art lives in `gfx/` as `.raw` + `.raw.pal` files.

### Adding content

- New verbs/synonyms: `verbs.txt`
- New strings: `strings.txt` (append, use next index)
- New state transitions: `states.c` — add entries to the relevant `ACTIONS_[STATE]` array
- New graphics: compress `.raw` → `.bin` with LZ4, register in `gfx.c`
