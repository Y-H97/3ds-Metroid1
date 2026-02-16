#!/usr/bin/env bash
# Exportiert alle LevelEditor-Lua-Dateien nach 3ds-cpp/romfs/maps (Linux/macOS).
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

echo "Exportiere LevelEditor Maps nach 3ds-cpp/romfs/maps ..."
python3 "$SCRIPT_DIR/export_maps.py"
echo
echo "Export erfolgreich."
