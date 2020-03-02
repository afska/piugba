# piugba

This is a version of PIU for the GBA. It's under development, so don't try to compile it yet: it won't work since the necessary images and audio files (`src/data/content`) are not here. A ROM creator will be provided in the final release.

## Install

### Windows

- Install `devKitPro` (v2.10.0) and clone the project. Use this file structure:
	* `gba`
		* `tools`
			* `devKitPro`
		* `projects`
			* `piugba`
- Configure environment variables:
	* `PATH` (add `{DEVKITARM}/bin` and `{DEVKITPRO}/tools/bin`)
	* (it's better to `export PATH=$PATH:{NEW_PATHS}` in `~/.bash_profile`)
- Run script:
	* `./configure.sh`
- Use `msys2` as shell when using `make`, and never move any folder ¯\_(ツ)_/¯

### VSCode

- Recommended plugins: `C/C++ Extensions`, `EditorConfig`
- Configuration:

```json
  "extensions.ignoreRecommendations": false,
  "terminal.integrated.shell.windows": "{PATH_TO_MSYS2_BASH.exe}",
  "terminal.integrated.shellArgs.windows": ["--login", "-i"],
  "C_Cpp.clang_format_style": "{ BasedOnStyle: Chromium }"
```

## Actions

### Commands

- `make clean`
- `make build`
- `make start`

### Build images

```bash
# use #FF00FF as transparency color
grit *.bmp -ftc -pS -gB8 -gT ff00ff -O shared.c
```

### Build music

```bash
ffmpeg -i file.mp3 -ac 1 -af 'aresample=18157' -strict unofficial -c:a gsm file.gsm
ffplay -ar 18157 file.gsm
```

### Build gba-sprite-engine

```bash
rm -rf cmake-build-debug ; mkdir cmake-build-debug ; cd cmake-build-debug ; cmake ./../ -G "Unix Makefiles" ; make ; cp engine/libgba-sprite-engine.a ../../piugba/libs/libgba-sprite-engine/lib/libgba-sprite-engine.a ; cd ../
```

## Versions

- `wgroeneveld/gba-sprite-engine`: Dec 18, 2019
  *->* Forked at: `rodri042/gba-sprite-engine`
- `pinobatch/gsmplayer-gba`: Feb 9, 2020
