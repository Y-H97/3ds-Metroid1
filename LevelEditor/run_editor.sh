#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if ! command -v love >/dev/null 2>&1; then
  echo "FEHLER: 'love' nicht gefunden. Bitte LÖVE installieren (https://love2d.org)." >&2
  exit 1
fi

echo "Starte Level Editor..."
cd "$SCRIPT_DIR"
love .
