# Lua → C++ Vergleich (aktualisiert, Februar 2026)

## Ziel
Dieser Vergleich stellt den aktuellen Stand zwischen der alten Lua-Version (`Metroidvania-main/Game`) und der 3DS-C++-Version (`3ds-cpp`) gegenüber und zeigt klar, welche Features bereits übertragen sind und welche nächsten Schritte den größten Mehrwert haben.

## Kurzfazit
- Die früher größten Lücken (Persistenz, Continue-Flow, Settings) sind in C++ inzwischen weitgehend geschlossen.
- Die wichtigsten offenen Punkte liegen jetzt eher in **Komfort/Polish**, **Audio**, **Performance-Logging** und **erweiterten Kartenfunktionen**.

---

## Status nach Funktionsbereich

### 1) Savegame & Continue
**Lua:**
- `savegame.lua` + Continue-Erkennung im Menü.

**C++ (Ist): ✅ umgesetzt + erweitert**
- Persistente Savegames auf SD (`savegame_slot1..3`).
- Continue-Verfügbarkeit wird pro aktivem Slot geprüft.
- Neues Spiel und Continue arbeiten slotbasiert.
- Legacy-Fallback für alte Datei (`savegame.dat` in Slot 1) vorhanden.

**Bemerkung:**
- C++ ist hier inzwischen funktional über Lua hinaus (3 Slots statt 1).

### 2) Settings-Persistenz
**Lua:**
- Umfangreiche `settings.lua` (FPS, Debug, Volumes, Profiling, Logger, Tuning).

**C++ (Ist): 🟡 teilweise umgesetzt**
- Persistiert: `debugEnabled`, `controlsSwapped`, `showFps` (`settings.dat`).
- Im Hauptmenü gibt es erklärende Hilfe je ausgewählter Option.

**Noch offen gegenüber Lua:**
- Kein FPS-Limit-Mode.
- Keine Audio-Settings.
- Keine erweiterten Profiling-/Logger-Tuning-Parameter.

### 3) Kartenlogik / Fog-of-War
**Lua:**
- Discovery/Visited-Logik vorhanden.

**C++ (Ist): ✅ umgesetzt**
- `visited`-Tracking aktiv.
- Karte zeigt besuchte Zellen + aktuelle Position.
- Visited-Status wird persistent pro Slot gespeichert.
- Reset des Kartenfortschritts im Hauptmenü vorhanden.

**Noch offen (optional):**
- Follow/Free-Modus (frei verschiebbare Kartenkamera).

### 4) Hauptmenü / UX
**Lua:**
- Einfaches Menü mit Main + Options.

**C++ (Ist): ✅ modernisiert**
- Strukturierteres 2-Screen-Menü (Top: Kontext/Hilfe, Bottom: Aktionen).
- Kategorien: Spielen / Optionen / Beenden.
- Slotwahl direkt im Spielen-Menü.
- Continue oben, Neues Spiel darunter.
- Lesbarkeit zuletzt erhöht (größere Texte).

### 5) Ingame-UI
**Lua:**
- Menüs/Debug/FPS vorhanden.

**C++ (Ist): ✅/🟡**
- Bottom-Tabs: Karte, Inventar, Einstellungen, Debug.
- Debug-Infos deutlich ausgebaut, inkl. scrollbarer Ansicht.
- Ingame-Einstellungen: FPS-Toggle + Rückkehr ins Hauptmenü.
- Inventar ist derzeit noch Placeholder („Aktuell leer“).

### 6) Performance/Profiler/Logging
**Lua:**
- Eigener Logger (`performance_log.txt/.csv`), Spike-Logs, periodisches Flush.

**C++ (Ist): 🔴 offen**
- Debug-Overlay vorhanden, aber kein persistentes Performance-Logging auf SD.

### 7) Audio
**Lua:**
- `soundVolume`, `musicVolume` in Settings.

**C++ (Ist): 🔴 offen**
- Kein Audio-System mit nutzbaren Menüoptionen.

---

## Priorisierte nächste Features (was du als Nächstes implementieren kannst)

### Priorität A (hoher Nutzen, moderater Aufwand)
1. **Performance-Logger (Debug-Build only)**
	- Frame-Zeit, Update-/Render-Zeit, Transition-Zeit, RAM-Hinweise periodisch nach SD schreiben.
	- Rotierende Log-Datei (wie Lua) zur Begrenzung der Dateigröße.

2. **Map-Qualität: Follow/Free-Modus**
	- Toggle in Karten-Tab: `Follow` (Spielerzentriert) vs. `Free` (DPad/Touch verschieben).
	- Optional kleiner Zoom-Level (2 Stufen reichen für MVP).

3. **Slot-Management-Polish im Spielen-Menü**
	- Zeige pro Slot kurz: `Save vorhanden / leer` + letzter Levelname (wenn vorhanden).

### Priorität B (mittelfristig)
4. **Inventar-MVP statt Placeholder**
	- Datenmodell für einfache Items/Flags.
	- Anzeige im Inventar-Tab (Liste + kurze Beschreibung).

5. **Settings-Ausbau (ohne Audio-Engine-Zwang)**
	- `fpsUnlimited` / Framecap-Option.
	- Optional: Debug-Detailebene (`basic` / `full`).

### Priorität C (nach Audio-Integration)
6. **Audio-System + Audio-Settings**
	- Musik-/SFX-Lautstärke im Menü.
	- Persistenz in `settings.dat` erweitern.

---

## Konkreter nächster Sprint (empfohlen)

### Sprint-Ziel
**Technisches Debugging und UX-Mehrwert ohne große Architektur-Risiken.**

### Umfang
1. Performance-Logger (ein/aus über Debug-Flag).
2. Map Follow/Free-Modus + einfache Kartenverschiebung.
3. Slot-Infozeile im Spielen-Menü (`Slot n`, `Save ja/nein`, optional letzter Levelname).

### Abnahmekriterien
- Logger schreibt Dateien zuverlässig auf SD und blockiert Gameplay nicht spürbar.
- Kartenmodus lässt sich umschalten und bleibt bedienbar.
- Spieler erkennt sofort, welcher Slot wirklich einen Fortschritt enthält.

---

## Gesamtbewertung
Die C++-Version hat die früheren Kernlücken zu Lua (Save, Continue, Settings-Basis, Discovery) inzwischen geschlossen und ist in mehreren Punkten weiter (3 Save-Slots, besser strukturiertes Hauptmenü). Die größte sinnvolle Lücke ist jetzt nicht mehr „Core-Gameplay“, sondern **Tooling/Transparenz (Performance-Logs)** plus **UX-Polish (Map/Slot-Info/Inventar-MVP)**.
