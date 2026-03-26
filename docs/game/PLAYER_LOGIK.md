# Player- und Sprunglogik

Diese Seite dokumentiert die neue Aufteilung der Spielerbewegung.
Ziel ist eine bessere Wartbarkeit: Sprungverhalten und allgemeine Bewegung
sind getrennt und koennen dadurch einzeln angepasst oder erweitert werden.

## Neue Modulstruktur

- `3ds-cpp/source/core/game_core.cpp`
  - delegiert das Spieler-Update nur noch an `playerlogic::updatePlayerMovement(...)`.
  - enthaelt damit keine detailreiche Sprung-/Kollisionslogik mehr.
- `3ds-cpp/source/gameplay/player/player_logic.h/.cpp`
  - orchestriert die Bewegungs-Pipeline pro Frame.
- `3ds-cpp/source/gameplay/player/jump_logic.h/.cpp`
  - kapselt die Sprungdetails: Bodensprung, Coyote-Time, Doppelsprung,
    variabler Sprung-Cut und Coyote-Timer-Update.
- `3ds-cpp/source/gameplay/player/movement_input.h/.cpp`
  - wandelt Rohinput (links/rechts) in horizontale Geschwindigkeit.
- `3ds-cpp/source/gameplay/player/collision_logic.h/.cpp`
  - kapselt X/Y-Kollisionen und Schraegkachel-Interaktionen.
- `3ds-cpp/source/gameplay/player/danger_logic.h/.cpp`
  - prueft Gefahrenkacheln (Tile-ID 2) getrennt von der Bewegung.

## Update-Ablauf pro Frame

1. `movement_input` setzt links/rechts in `vx` um.
2. `jump_logic` verarbeitet Sprung-Input und variablen Jump-Cut.
3. Gravitation wird angewendet.
4. `collision_logic` loest X-Kollisionen, danach Y-Kollisionen.
5. `jump_logic` aktualisiert Coyote-Time nach Grounding-Zustand.
6. `danger_logic` prueft, ob der Spieler in Gefahr steht.

## Warum diese Aufteilung hilft

- Aenderungen an Sprungwerten bleiben lokal in `jump_logic`.
- Kollisionstuning (Step-Height, Tile-Pruefung) bleibt lokal in `collision_logic`.
- `GameCore` bleibt als Fassade schlank und ist leichter lesbar.
- Neue Bewegungsfeatures (z. B. Wall-Jump) koennen als weiteres Modul
  neben `jump_logic` angelegt werden, ohne den Core aufzublaehen.

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/game/README|Game Überblick]]
- [[docs/game/ARCHITECTURE|Game-Architektur]]
- [[docs/game/RUNTIME_FLOW|Game-Laufzeitfluss]]
- [[docs/workspace/ORDNERSTRUKTUR_GESAMTPROJEKT|Ordnerstruktur Gesamtprojekt]]
