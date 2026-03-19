# Game-Laufzeitfluss

## Start
1. Init von 3DS/GFX/romfs.
2. `MainMenuController` und `GameplayScene` werden initialisiert.
3. Persistente Settings werden geladen und auf Menü/Gameplay angewandt.

## Hauptloop
- Input lesen (`hidScanInput`).
- Wenn `APP_MAIN_MENU`:
  - Menü verarbeiten.
  - Aktionen abrufen (`consumeAction`).
  - Aktionen in Gameplay-Operationen übersetzen:
    - Neues Spiel
    - Fortsetzen
    - Kartenfortschritt zurücksetzen
- Wenn `APP_GAME`:
  - Gameplay Input/Update.
  - Items einsammeln und Bottom-UI (Inventar) verarbeiten. Das Inventar kann
    per Richtungs-Tasten oder `Y`/Touch umgeschaltet werden.
  - Rückkehr ins Menü oder Exit prüfen.

## Render
- Menüzustand: `menu.renderTop` + `menu.renderBottom`
- Gameplayzustand: `gameplay.renderTop` + `gameplay.renderBottom`

## Shutdown
- Gameplay persistiert aktuellen Stand (Checkpoint/Fallback).
- Renderer/Subsysteme werden sauber beendet.

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/game/README|Game Überblick]]
- [[docs/game/ARCHITECTURE|Game-Architektur]]
- [[docs/game/SAVE_AND_SETTINGS|Save-, Slot- und Settings-System]]
- [[docs/simulator/README|Simulator Überblick]]
