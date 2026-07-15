# REAPER Terminal

REAPER Terminal is a small, native C++ REAPER extension that launches the user's
installed terminal application in the active REAPER project directory and exports
useful REAPER context to that terminal.

It is intentionally **not** an embedded or docked terminal emulator. It does not
recreate a terminal inside REAPER, embed an AI client, or alter the REAPER project.
Instead, it gives REAPER a JetBrains-style "open terminal here" workflow using the
terminal already installed on each supported platform.

The action is named **REAPER Terminal: Open native terminal** and an
**Extensions > Open REAPER Terminal** menu item is also registered.

## Example uses

Opening a terminal directly in the current project directory can be useful for:

- inspecting, searching, renaming, copying, or organizing project and media files;
- running `git` commands for versioned `.rpp` projects, scripts, presets, or related
  documentation;
- opening project files, notes, scripts, or metadata in terminal editors such as
  Vim;
- using `ffmpeg`, `ffprobe`, SoX, or other audio utilities on project media;
- running project-specific shell scripts, Python tools, build commands, or batch
  processing workflows;
- synchronizing or backing up project files with tools such as `rsync`;
- calculating checksums or comparing files during delivery and archival work;
- inspecting the REAPER resource directory and developing or testing ReaScripts and
  native extensions;
- using any command-line assistant or other interactive CLI tool with the current
  REAPER project context available through environment variables.

REAPER Terminal does not implement these workflows itself. It simply opens the
user's normal terminal in the right place and makes REAPER context available to
whatever tools the user chooses.

## Behavior

For a saved project, the terminal starts in the directory containing its `.rpp`
file. An unsaved project starts in REAPER's effective media directory (normally
`~/Documents/REAPER Media`), with that default path used if REAPER does not
return a usable directory.

Every launched shell receives tool-neutral context:

```text
REAPER_TERMINAL=1
REAPER_TERMINAL_VERSION=0.2.0
REAPER_VERSION=<host version>
REAPER_PROJECT_FILE=<saved project or empty>
REAPER_PROJECT_DIR=<starting directory>
REAPER_RESOURCE_DIR=<REAPER resource directory>
REAPER_API_DOCS=https://www.reaper.fm/sdk/reascript/reascripthelp.html
REAPER_EXTENSION_SDK=https://www.reaper.fm/sdk/plugin/plugin.php
REAPER_SDK_SOURCE=https://github.com/justinfrankel/reaper-sdk
TERM_PROGRAM=REAPER Terminal
TERM_PROGRAM_VERSION=0.2.0
```

The terminal also prints a compact startup header containing the REAPER and
extension versions, working directory, saved project (or `(unsaved)`), and the
three official API/SDK links. The header is informational and tool-neutral.

These variables work with shells, scripts, editors, build tools, and any current
or future CLI assistant. No `AGENTS.md`, repository file, prompt, or
assistant-specific setting is generated.

To start a program automatically after the context and header are initialized,
choose **Extensions > REAPER Terminal settings...** and enter a startup command
such as `vim`, `git status`, a project script, or another command-line tool. The
setting is stored persistently by REAPER. It is interpreted as a shell command,
so arguments are supported. When the startup program exits, the terminal remains
open at the normal login shell. Clear the setting for the default shell-only
behavior. The setting is also available as **REAPER Terminal: Configure startup
command** in the Actions list.

### CLI assistants

Terminal scrollback is not automatically included in a CLI assistant's model
context. To start Codex and direct it to the exported REAPER metadata, use this
startup command:

```sh
codex "You are running inside REAPER Terminal. Inspect the REAPER_* environment variables to learn the current REAPER project context before helping me."
```

This gives Codex an initial instruction to inspect the same tool-neutral context
that is available to every program launched by REAPER Terminal.

## Platform launchers

- Linux tries `kgx`, GNOME Terminal, Konsole, Xfce Terminal,
  `x-terminal-emulator`, then xterm. Set `REAPER_TERMINAL_COMMAND` to an
  executable name/path to override detection. A shell bootstrap explicitly
  exports metadata, so it also works with single-instance terminal services.
- macOS opens a new Terminal.app instance at the working directory.
- Windows prefers Windows Terminal and falls back to a new PowerShell console.
  A dedicated Unicode environment block carries the metadata.

Linux was built and tested locally with REAPER 7.77 and GNOME Console 48.0.1.
macOS and Windows are implemented but have not been built on those platforms.

## Dependencies

- REAPER extension SDK — permissive Cockos source license.
- WDL/SWELL module stub — zlib-style permissive license.
- Operating-system terminal application — runtime component selected above.

There is no terminal-emulation dependency and no Electron, browser, Node.js,
Python, or AI dependency. The old libvterm checkout may remain in existing
development trees but is no longer compiled or linked.

Fetch the two build dependencies before configuring:

```sh
mkdir -p third_party
git clone https://github.com/justinfrankel/reaper-sdk third_party/reaper-sdk
git clone https://github.com/justinfrankel/WDL third_party/WDL
```

## Build and install on Linux

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
mkdir -p ~/.config/REAPER/UserPlugins
cp build/reaper_reaper_terminal.so ~/.config/REAPER/UserPlugins/
```

Restart REAPER after replacing an already-loaded extension.

## macOS

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --parallel
ctest --test-dir build --output-on-failure
mkdir -p ~/Library/Application\ Support/REAPER/UserPlugins
cp build/reaper_reaper_terminal.dylib ~/Library/Application\ Support/REAPER/UserPlugins/
```

## Windows

Use Visual Studio 2022/MSVC for REAPER ABI compatibility:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
Copy-Item build\Release\reaper_reaper_terminal.dll "$env:APPDATA\REAPER\UserPlugins\"
```

## Validation

Automated tests verify all generic metadata and official documentation links.
The Linux live smoke test verified that REAPER loads the extension and that an
already-running GNOME Console service creates a new login shell with the full
metadata environment and requested working directory.

Manual checks:

- Saved project opens at its containing directory.
- Unsaved project opens in the configured/default `Documents/REAPER Media` directory.
- `env | grep '^REAPER_'` shows the metadata above.
- Codex, Vim, Git interactive commands, and other terminal programs behave as
  they do in the user's normal terminal.
- Opening or closing the terminal does not change the REAPER project.
