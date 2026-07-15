# REAPER Terminal plan

1. Register one native REAPER action and Extensions menu entry.
2. Resolve the active saved project directory, falling back to REAPER's media
   directory (normally `Documents/REAPER Media`) for unsaved projects.
3. Launch the user's native terminal through a platform abstraction.
4. Bootstrap the shell with generic REAPER metadata and official API/SDK links.
5. Keep assistant integrations optional and outside the terminal extension.
6. Build/test on Linux; retain careful but explicitly unverified macOS/Windows
   launchers until native builds or CI are available.
