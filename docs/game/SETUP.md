# Game Setup (Windows & Linux)

## Ziel
Build der 3DS-Runtime (`3ds-cpp.3dsx` + `.smdh`).

## Voraussetzungen
- devkitARM + libctru
- citro2d / citro3d
- Umgebungsvariable `DEVKITARM`

## Linux
1. devkitPro/devkitARM installieren.
2. `DEVKITARM` setzen.
3. Build:
   - `cd 3ds-cpp`
   - `make`

## Windows
### Variante A (klassisch)
1. devkitPro/devkitARM installieren.
2. Build in `3ds-cpp`:
   - `make`

### Variante B (lokaler Spiegel-Workflow)
- `3ds-cpp\\build_local.bat`

Ablauf:
1. Level-Export starten
2. Workspace lokal nach `%LOCALAPPDATA%\\Temp\\3ds-cpp-build` spiegeln
3. `make clean` + `make`
4. Artefakte zurück nach `3ds-cpp`

## Artefakte
- `3ds-cpp/3ds-cpp.3dsx`
- `3ds-cpp/3ds-cpp.smdh`
