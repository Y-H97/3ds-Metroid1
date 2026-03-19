# Simulator Steuerung

## Window-Simulator (Windows)

### Top-Screen / Gameplay
- Bewegung: `Pfeiltasten`
- Springen: `SPACE`
- Zurück ins Hauptmenü: `ESC` (entspricht `Select`)

### Bottom-Screen / DPad-Ersatz
- Links/Rechts/Hoch/Runter: `W/A/S/D`
- Alternativ:
  - Links/Rechts: `Q/E`
  - Hoch/Runter: `I/K`

### Menüsteuerung
- Bestätigen: `Enter` (A/Y)
- Zurück: `Backspace` (B)
- Touch-Ersatz: Linksklick auf Bottom-Screen

### Bottom-Tabs im Gameplay
- Wechsel: `A/D`, `Q/E` oder Klick auf Tab-Leiste
- `Y` toggelt im Inventar den ausgewählten Eintrag (alternativ Klick/Tap auf
  einen Gegenstand).
## Console-Simulator (Windows/Linux)

### Start
- Standardmap: `test.json`
- Optional mit Pfad: `...run_simulator_console... <map.json>`

### Befehle
- `help` – Hilfe anzeigen
- `status` – Spielerstatus anzeigen
- `view [w h]` – ASCII-Viewport
- `step [n] [keys]` – Frames simulieren (`keys`: `l r j`)
- `hold <l|r|j> <on|off>` – Dauerinput setzen
- `tp <x> <y>` – Position setzen
- `reset` – zum Spawn zurück
- `map <pfad>` – andere Map laden
- `quit` – beenden

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/simulator/README|Simulator Überblick]]
- [[docs/simulator/SETUP|Simulator Setup]]
- [[docs/quickstart/tester|Quick Start – Tester]]
