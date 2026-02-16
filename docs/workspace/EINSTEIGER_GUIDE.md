# Einsteiger-Guide (Technik ohne Vorwissen)

Diese Seite erklärt den Workspace so, dass auch Nicht-Programmierer verstehen, **welcher Teil was macht**.

## 1) Das große Bild
- **Spiel-Logik (C++)** liegt in [3ds-cpp/source](../../3ds-cpp/source).
- **Desktop-Simulator** (zum schnellen Testen am PC) liegt in [simulator](../../simulator).
- **Level-Editor** (Karten bauen) liegt in [LevelEditor](../../LevelEditor).
- **Export** wandelt Editor-Dateien in Spielkarten um: von [LevelEditor/level](../../LevelEditor/level) nach [3ds-cpp/romfs/maps](../../3ds-cpp/romfs/maps).
- **Visualisierung** der Zusammenhänge: [ARCHITEKTUR_UEBERSICHT.md](ARCHITEKTUR_UEBERSICHT.md).

## 2) Was macht welcher Bereich?
- **Spielkern** in [3ds-cpp/source/core](../../3ds-cpp/source/core): Bewegung, Physik, Kollision, Weltstruktur, Einstellungen.
- **Gameplay-Ablauf** in [3ds-cpp/source/gameplay](../../3ds-cpp/source/gameplay): Input, Raumwechsel, Respawn, Speichern.
- **Menüsystem** in [3ds-cpp/source/menu](../../3ds-cpp/source/menu): Hauptmenü, Optionen, Handbuchseiten.
- **Textausgabe** in [3ds-cpp/source/ui](../../3ds-cpp/source/ui): Zeichnet Texte auf den Screens.
- **Simulator-Fenster** in [simulator/window_main_win32.cpp](../../simulator/window_main_win32.cpp): spiegelt Spiel-/Menülogik auf Windows.
- **Simulator-Konsole** in [simulator/main.cpp](../../simulator/main.cpp): Test per Textbefehlen.

## 3) Typischer Arbeitsablauf
1. Level im Editor bauen: [LevelEditor/main.lua](../../LevelEditor/main.lua)
2. Level exportieren: [LevelEditor/export_maps.py](../../LevelEditor/export_maps.py) oder [LevelEditor/export_maps.ps1](../../LevelEditor/export_maps.ps1)
3. Simulator starten: [simulator/run_simulator_window.bat](../../simulator/run_simulator_window.bat)
4. Optional 3DS-Build: [3ds-cpp/build_local.bat](../../3ds-cpp/build_local.bat)

## 4) Wo finde ich Einstieg je Rolle?
- Allgemeine Quickstarts: [docs/quickstart/README.md](../quickstart/README.md)
- Entwickler: [docs/quickstart/developer.md](../quickstart/developer.md)
- Level-Design: [docs/quickstart/level-designer.md](../quickstart/level-designer.md)
- Testen: [docs/quickstart/tester.md](../quickstart/tester.md)

## 5) Häufige Fragen (kurz)
- **„Warum sehe ich im Spiel keine neuen Level?“**
  Meist wurde nicht exportiert. Prüfe, ob in [3ds-cpp/romfs/maps](../../3ds-cpp/romfs/maps) aktuelle `.json`-Dateien liegen.
- **„Wo wird gespeichert?“**
  Spielstände/Settings liegen je nach Laufumgebung in den jeweils vorgesehenen Save-Pfaden (3DS/Simulator).
- **„Was ändere ich, wenn nur UI anders aussehen soll?“**
  Meist in [3ds-cpp/source/menu/views](../../3ds-cpp/source/menu/views) oder [3ds-cpp/source/gameplay/render](../../3ds-cpp/source/gameplay/render).

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/workspace/ARCHITEKTUR_UEBERSICHT|Architektur-Übersicht]]
- [[docs/game/README|Game Überblick]]
- [[docs/simulator/README|Simulator Überblick]]
- [[docs/level-editor/README|LevelEditor Überblick]]
