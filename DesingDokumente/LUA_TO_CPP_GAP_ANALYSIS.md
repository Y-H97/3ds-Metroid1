# Lua → C++ Vergleich (Stand heute)

## Ziel
Diese Datei vergleicht die alte Lua-Version (`Metroidvania-main/Game`) mit der aktuellen 3DS-C++-Version (`3ds-cpp`) und zeigt, was bereits übertragen ist und was noch fehlt.

## Bereits übertragen (wichtigste Punkte)
- Hauptmenü mit Spielstart, Einstellungen, Beenden.
- Laden vom letzten Checkpoint aus dem Hauptmenü.
- Ingame-Bottom-Tabs: Karte, Inventar, Einstellungen, Debug.
- Debug-Info Toggle im Hauptmenü + umfangreiche Debug-Anzeige ingame.
- FPS-Anzeige Toggle in den Ingame-Einstellungen.
- Welt-/Raumübergänge über `world.json` + Spatial-Map.
- Checkpoint-System zur Laufzeit (Raumwechsel-Checkpoint + Respawn).
- Slopes (Tile-basiert) in Rendering und Kollision.
- Steuerungsprofil oben/unten inkl. Tausch-Option im Menü.

## Noch nicht übertragen / funktionale Lücken

### 1) Persistentes Savegame (hoch)
Lua:
- Speichert `savegame.lua` mit `level, gridX, gridY, x, y`.
- Menü erkennt Savegame und bietet "Weiter".

C++ aktuell:
- Checkpoint nur im RAM (Session-gebunden).
- Nach Neustart kein echtes "Weiter" von Disk.

Empfehlung:
- `romfs` ist read-only; daher Save in SD-Write-Pfad (z. B. über 3DS FS API).
- Kleines JSON/TXT Save-Format mit genau den Lua-Feldern.

### 2) Persistente Settings (hoch)
Lua:
- `Systems/settings.lua` mit `showFPS`, `debugInfo`, FPS-Limit, Cachegröße, Audio-Lautstärken.

C++ aktuell:
- Optionen sind derzeit session-lokal.
- Kein Laden/Speichern von Settings-Datei.

Empfehlung:
- Settings-Datei auf SD speichern (z. B. `settings.json`).
- Beim Start laden, im Menü ändern, sofort/sicher persistieren.

### 3) "Weiter"-Flow wie Lua-Menü (mittel-hoch)
Lua:
- Eigenes Continue mit Savegame-Existenzprüfung im Menü.

C++ aktuell:
- "Letzten Speicherpunkt laden" existiert, aber ohne Savegame-Existenz von Disk.

Empfehlung:
- Menüeintrag dynamisch aktiv/deaktiviert je nach Save-Datei.
- Optional Label wie "Weiter (kein Save)".

### 4) Kartenlogik (Fog-of-War / Discovery) (mittel)
Lua:
- `visited`-Tracking; Map zeigt primär entdeckte Zellen.
- Map-Ansicht mit Follow/Free-Modus + Drag/Scroll.

C++ aktuell:
- World-Map ist vorhanden, aber ohne Discovery/Fog-of-War.
- Kein Follow/Free-Modus für die Map.

Empfehlung:
- `visited`-Set in Gameplay führen.
- Nur besuchte Cells anzeigen (aktueller Cell immer sichtbar).

### 5) Performance-/Profiler-Infrastruktur (mittel)
Lua:
- Umfangreicher Profiler + periodisches Logging (`performance_log.txt`), Spike-Logs.
- Async Build für Render/Physics.

C++ aktuell:
- Debug-Infos sind gut, aber kein persistentes Performance-Logging.
- Kein analoger Async-Pipeline-Ansatz auf Dateiebene.

Empfehlung:
- Optionales Dev-Logging (nur Debug-Build), z. B. Frame ms, Transition ms, Mem.

### 6) Audio-Settings (niedrig-mittel)
Lua:
- `soundVolume`, `musicVolume` vorhanden.

C++ aktuell:
- Keine Audio-Optionen im Menü.

Empfehlung:
- Erst sinnvoll bei/ nach Audio-System-Integration.

## Technische Unterschiede (bewusst oder Migration-bedingt)
- Lua nutzte freie `objects` für Geometrie; C++ ist jetzt absichtlich tile-only.
- Lua-Pfadlogik basierte auf Love2D-Dateisystem; C++ muss 3DS-spezifisch zwischen read-only `romfs` und writable SD unterscheiden.

## Priorisierte nächste Implementierungsschritte
1. Persistentes Savegame (SD) + echtes Continue im Menü.
2. Persistente Settings (mind. `debug`, `fps overlay`, Steuerungs-Tausch).
3. Savegame-Existenz im Hauptmenü UI sichtbar machen.
4. Optional: Map-Discovery (`visited`) wie in Lua.
5. Optional: Dev-Performance-Logging.

## Konkreter MVP-Vorschlag (nächster Sprint)
- Savegame-Datei lesen/schreiben (JSON).
- Beim Checkpoint autosave auslösen.
- Hauptmenü: "Weiter" nur aktiv, wenn Save vorhanden.
- Beim Start: gespeicherte Settings laden (debug/fps/controls swap).

---
Ergebnis: Die C++-Version ist gameplay-seitig bereits weit, aber die größten Lua-Features, die noch fehlen, sind Persistenz (Save + Settings) und der vollständige Continue-Flow.
