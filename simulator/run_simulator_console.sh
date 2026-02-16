#!/usr/bin/env bash
set -euo pipefail

SIM_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SIM_ROOT/.." && pwd)"
CPP_ROOT="$PROJECT_ROOT/3ds-cpp"

BUILD_ONLY=0
if [[ "${1:-}" == "--build-only" ]]; then
  BUILD_ONLY=1
  shift
fi

cd "$CPP_ROOT"
mkdir -p build/simulator
OUT="build/simulator/metroid_sim_console"

CXX="${CXX:-g++}"
echo "Baue Console-Simulator mit $CXX..."
"$CXX" -std=gnu++17 -O2 -Wall -Wextra -o "$OUT" \
  "$SIM_ROOT/main.cpp" \
  "$CPP_ROOT/source/core/game_core.cpp"

if [[ $BUILD_ONLY -eq 1 ]]; then
  echo "Build erfolgreich: $OUT"
  exit 0
fi

echo
echo "Starte Console-Simulator..."
echo
if [[ $# -eq 0 ]]; then
  "$OUT" "$CPP_ROOT/romfs/maps/test.json"
else
  "$OUT" "$@"
fi
