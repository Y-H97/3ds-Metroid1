#!/usr/bin/env python3
# Exportiert Lua-Leveldaten des Editors nach JSON für das C++-Spiel.
import argparse
import json
import re
import sys
from pathlib import Path


def parse_room_lua(path: Path):
    # Liest Raum-Lua-Datei und extrahiert ein rechteckiges Tile-Grid.
    content = path.read_text(encoding="utf-8")
    lines = content.splitlines()

    in_grid = False
    has_explicit_grid = False
    in_top_level = False
    rows = []

    for line in lines:
        if not in_grid and re.match(r"^\s*grid\s*=\s*\{", line):
            in_grid = True
            has_explicit_grid = True
            continue

        if not has_explicit_grid and not in_top_level and re.match(r"^\s*return\s*\{", line):
            in_top_level = True
            continue

        if in_grid and re.match(r"^\s*\},\s*$", line):
            break

        if in_top_level and re.match(r"^\s*\}\s*$", line):
            break

        if (in_grid or in_top_level) and re.match(r"^\s*\{", line):
            if "=" in line:
                continue
            nums = re.findall(r"-?\d+", line)
            if nums:
                rows.append([int(n) for n in nums])

    if not rows:
        raise ValueError(f"Kein Grid in {path} gefunden")

    width = len(rows[0])
    for idx, row in enumerate(rows):
        if len(row) != width:
            raise ValueError(
                f"Uneinheitliche Zeilenbreite in {path} (Zeile {idx}, erwartet {width}, gefunden {len(row)})"
            )

    height = len(rows)
    tiles = [value for row in rows for value in row]

    return {
        "name": path.stem,
        "width": width,
        "height": height,
        "tiles": tiles,
    }


def parse_world_lua(path: Path):
    # Liest world.lua und extrahiert Raumzellen + Checkpoint-Flags.
    content = path.read_text(encoding="utf-8")

    cell_matches = re.finditer(r"\['(-?\d+),(-?\d+)'\]\s*=\s*'([^']+)'", content)
    cells = [
        {"x": int(m.group(1)), "y": int(m.group(2)), "level": m.group(3)}
        for m in cell_matches
    ]
    cells.sort(key=lambda c: (c["y"], c["x"]))

    checkpoint_matches = re.finditer(r"\['(-?\d+,-?\d+)'\]\s*=\s*true", content)
    checkpoints = {m.group(1): True for m in checkpoint_matches}

    return {"cells": cells, "checkpoints": checkpoints}


def validate_room_json(room_obj: dict, source: Path):
    # Sicherstellt, dass width*height exakt zur Tile-Anzahl passt.
    expected = int(room_obj["width"]) * int(room_obj["height"])
    actual = len(room_obj["tiles"])
    if actual != expected:
        raise ValueError(f"Ungültige Raumdaten in {source}: tiles={actual}, erwartet={expected}")


def validate_world_json(world_obj: dict, available_rooms: set, source: Path):
    # Prüft, ob alle in world.lua referenzierten Räume tatsächlich exportiert wurden.
    for cell in world_obj.get("cells", []):
        level = str(cell.get("level", ""))
        if level not in available_rooms:
            raise ValueError(f"Welt referenziert unbekannten Raum '{level}' in {source}")


def write_json(path: Path, obj: dict):
    # Schreibt kompaktes JSON (ohne unnötige Leerzeichen).
    path.write_text(json.dumps(obj, ensure_ascii=False, separators=(",", ":")), encoding="utf-8")


def main():
    # End-to-End Export: Räume + Welt laden, validieren und schreiben.
    parser = argparse.ArgumentParser(description="Exportiert LevelEditor Lua-Dateien nach JSON")
    parser.add_argument("--level-dir", default=str(Path(__file__).parent / "level"))
    parser.add_argument("--maps-dir", default=str(Path(__file__).parent.parent / "3ds-cpp" / "romfs" / "maps"))
    args = parser.parse_args()

    level_dir = Path(args.level_dir)
    maps_dir = Path(args.maps_dir)

    if not level_dir.exists():
        raise FileNotFoundError(f"Level-Verzeichnis nicht gefunden: {level_dir}")

    maps_dir.mkdir(parents=True, exist_ok=True)

    files = sorted(level_dir.glob("*.lua"), key=lambda p: p.name.lower())
    room_files = [f for f in files if f.name.lower() != "world.lua"]
    world_file = next((f for f in files if f.name.lower() == "world.lua"), None)

    exported_rooms = set()
    count = 0

    for room_file in room_files:
        room_obj = parse_room_lua(room_file)
        validate_room_json(room_obj, room_file)
        out_path = maps_dir / f"{room_file.stem}.json"
        write_json(out_path, room_obj)
        exported_rooms.add(room_file.stem)
        count += 1

    if world_file is not None:
        world_obj = parse_world_lua(world_file)
        validate_world_json(world_obj, exported_rooms, world_file)
        write_json(maps_dir / "world.json", world_obj)
        count += 1

    print(f"Export fertig. {count} Datei(en) nach '{maps_dir}' geschrieben.")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"FEHLER: {exc}", file=sys.stderr)
        sys.exit(1)
