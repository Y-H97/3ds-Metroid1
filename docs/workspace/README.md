# Workspace-Struktur

## Top-Level
- `3ds-cpp/` – C++-Game-Runtime (3DS Build + Assets + Quellcode)
- `simulator/` – Desktop-Simulatoren (Window/Console)
- `LevelEditor/` – Lua/LÖVE-Editor + Export
- `docs/` – Bereichsdokumentation (Game, Simulator, LevelEditor, Tooling)
- `DesingDokumente/` – zusätzliche Design-/Migrationsnotizen

## Datenfluss
- Editor-Quellen: `LevelEditor/level/*.lua`
- Exportziel: `3ds-cpp/romfs/maps/*.json`
- Nutzung: Runtime (`3ds-cpp/source`) und Simulator (`simulator/`)

## Einstieg ohne Vorwissen
- `EINSTEIGER_GUIDE.md` – einfache Modulübersicht, Datenfluss und typische Abläufe.
- `ARCHITEKTUR_UEBERSICHT.md` – visuelle Gesamtarchitektur mit Diagramm.

## Muss-Checkliste vor Abschluss
- [ ] Antworten/Änderungstexte auf Deutsch
- [ ] Neuer/geänderter Code ausführlich auf Deutsch kommentiert
- [ ] Betroffene Doku angepasst
- [ ] Obsidian-Querverweise ergänzt
- [ ] Build/Test kurz validiert oder begründet ausgelassen

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/workspace/EINSTEIGER_GUIDE|Einsteiger-Guide]]
- [[docs/workspace/ARCHITEKTUR_UEBERSICHT|Architektur-Übersicht]]
- [[docs/quickstart/README|Quick Start nach Rolle]]
