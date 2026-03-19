# Quick Start – Tester

## Ziel (5 Minuten)
- aktuellen Stand schnell starten
- Basis-Regression prüfen
- Befunde reproduzierbar dokumentieren

## Smoke-Test Ablauf
1. Build sicherstellen:
   - `simulator\run_simulator_window.bat --build-only`
2. Simulator starten:
   - `simulator\run_simulator_window.bat`
3. Menü prüfen:
   - Slot wechseln, Neues Spiel, Fortsetzen, Optionen.
4. Gameplay prüfen:
   - Bewegung/Sprung, Raumwechsel, Rückkehr ins Menü (`ESC`).
   - Items aufsammeln (z.B. Doppelsprung) und Bottom-UI öffnen, Status umschalten
     (`Y` oder Touch). Experimente im SafeRoom ausführen, um Spawn-Clamp zu
     prüfen.
5. Persistenz prüfen:
   - Checkpoint erreichen, schließen, neu starten, `Fortsetzen` testen.

## Bug-Report Mindestdaten
- Buildzeitpunkt/Branch
- getesteter Slot
- Karte/Raum (falls bekannt)
- erwartetes vs. tatsächliches Verhalten
- Repro-Schritte (nummeriert)

## Zusätzlicher Konsolentest (optional)
- `simulator\run_simulator_console.bat`
- Für schnelle Core-Verifikation ohne Fenster-UI.

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/simulator/CONTROLS|Simulator Steuerung]]
- [[docs/simulator/SETUP|Simulator Setup]]
- [[docs/game/RUNTIME_FLOW|Game-Laufzeitfluss]]
- [[docs/quickstart/README|Quick Start nach Rolle]]
