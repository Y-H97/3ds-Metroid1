# AI-Regeln für dieses Projekt

Dieses Dokument definiert verbindliche Arbeitsregeln für KI-gestützte Entwicklung in diesem Repository.

## Ziel
- Konsistente Zusammenarbeit über mehrere Sessions.
- Einheitliche Sprache (Deutsch).
- Kontinuierliche, verlinkte Dokumentation.

## Verbindliche Regeln

### 1) Sprache
- Antworten, Kommentare, Commit-nahe Beschreibungen und Doku-Texte standardmäßig auf Deutsch.
- Nur wenn ausdrücklich gewünscht: Ausgabe in einer anderen Sprache.

### 1.1) Code-Kommentare (verpflichtend)
- Neuer oder geänderter Code muss **ausführlich** mit deutschen Kommentaren versehen werden.
- Kommentare sollen auch für Einsteiger verständlich sein (kein unnötiger Fachjargon ohne Erklärung).
- Mindestens dokumentieren:
  - Zweck von Datei/Modul
  - Zweck zentraler Funktionen/Methoden
  - wichtige Eingaben/Ausgaben/Zustandsänderungen
  - besondere Logik (z. B. Kollision, Persistenz, Übergänge, Sonderfälle)
- Ausnahmen nur bei rein trivialen Einzeilern oder wenn Kommentare technisch keinen Mehrwert bieten.

### 2) Dokumentation fortführen
- Bei jeder relevanten Codeänderung prüfen, ob Doku angepasst werden muss.
- Neue Seiten in die Hauptnavigation aufnehmen:
  - [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
  - ggf. Bereichs-README ergänzen.
- Jede neue/angepasste Doku mit „Verwandte Dokus (Obsidian)“ verlinken.

### 3) Obsidian-Linkstandard
- Für interne Dokuverweise immer Wiki-Links verwenden, z. B.:
  - `[[docs/GESAMTDOKU]]`
  - `[[docs/game/ARCHITECTURE|Game-Architektur]]`
- Ziel: Von jeder Seite aus in 1–2 Klicks zu angrenzenden Themen kommen.

### 4) Arbeitsstil bei Änderungen
- Erst bestehende Struktur respektieren, dann minimal-invasiv ändern.
- Keine unnötigen Umbenennungen/Refactors ohne klaren Mehrwert.
- Bei größeren Änderungen kurz dokumentieren:
  - Was wurde geändert?
  - Warum?
  - Welche Doku wurde angepasst?

### 5) Qualitätssicherung
- Nach Änderungen möglichst passenden Build/Test/Export ausführen.
- Wenn etwas nicht getestet werden kann: transparent benennen.

## 6) Python & Workspace-Abhängigkeiten
- Lokale Python-Skripte (z. B. `check_wiki_links.py`, `export_maps.py`) nutzen Workspace-Python (`.venv`).
- Bei manueller Auführung lokal:
  - Zuerst `configure_python_environment` nutzen, um korrektes Python zu setzen.
  - Oder: `.venv/Scripts/python.exe <script>` auf Windows direkt aufrufen.
- PowerShell-Wrapper (z. B. `scripts/*.ps1`) sind optimiert, um automatisch `.venv` zu suchen.
- CI unter GitHub Actions nutzt `python3` + `pip install -r requirements-dev.txt` bei Bedarf.

## 7) Validierungswerkzeuge (vor Commit/PR)
Diese Checks laufen lokal oder in der CI:

| Tool | Zweck | Trigger | Befehl |
|---|---|---|---|
| `check_wiki_links.py` | Validiert Obsidian-Links, findet Orphan-Docs | CI: `docs/**` ändern | `python scripts/check_wiki_links.py` |
| `check_german_comments.py` | Prüft deutsche Kommentare in geänderten Dateien | CI: `main` Branch | `python scripts/check_german_comments.py` |
| `export_maps.py` | Konvertiert LevelEditor-Lua nach JSON, validiert | CI: vor Code-Checks | `python LevelEditor/export_maps.py` |

**Best Practice für Entwickler:**
1. Vor Commit lokal die passenden Checks laufen lassen:
   - Code-Änderungen: `export_maps.py` (falls Level betroffen), dann `check_german_comments.py`
   - Doku-Änderungen: `check_wiki_links.py`
2. Task-Shortcuts in VS Code nutzen (siehe `.vscode/tasks.json`):
   - "Check: Deutsche Kommentare"
   - "Check: Obsidian Wiki‑Links"
3. Nach dem Commit: GitHub CI „grün" machen, PRs müssen passieren.

## 8) CI/GitHub-Workflows
- **Datei:** `.github/workflows/*.yml`
- **ci.yml:**
  - Trigger: `push` zu `main` und `pull_request` (außer `docs/`, `LevelEditor/`)
  - Jobs: `export-maps` → `german-comments` (sequenziell)
  - Prüft: LevelEditor-Export und deutsche Kommentare in Quelldateien
- **docs-check.yml:**
  - Trigger: Änderungen in `docs/**`
  - Prüft: Obsidian-Wiki-Links und Orphan-Seiten
- Workflows müssen grün sein, bevor PR gemerged wird (siehe `PULL_REQUEST_TEMPLATE.md`).

## 9) Externe Workspace-Ordner (außerhalb des Repos)
Diese Ordner gehören nicht zum Git-Repository, sind aber notwendig:

| Ordner | Rolle | Konfiguration | Notizen |
|---|---|---|---|
| `c:\devkitPro` | 3DS-Toolchain | `DEVKITARM` Env-Variable, pacman/MSYS2 | Wird vom Makefile erwartet; Setup in [[docs/game/DEVKITARM_INSTALLATION]] |
| `mingw64` (WinLibs) | C++-Compiler für Simulator | Im PATH oder WinLibs-Ordner in Downloads | Fallback-Logik in `simulator/run_simulator_window.bat` |
| `cpp3ds-master` (Optional) | Externe Referenzlib | Aktuell nicht als Build-Abhängigkeit aktiv | Für zukünftige Refactors interessant |

**Regel:** Wenn extern installiert, mit Workspace-Doku verlinken (z. B. Pfade in Task-Beschreibungen).

## 10) Commit-Standards
- **Branch:** Feature-Branches aus `main` mit Präfix (z. B. `feature/`, `fix/`, `docs/`)
- **Message-Format:**
  ```
  [BEREICH] Kurzbeschreibung (Deutsch, Imperativ)
  
  - Was wurde geändert?
  - Falls Doku: welche Seiten angepasst?
  - Falls Quellcode: Kommentare vorhanden?
  ```
  Beispiele:
  - `[docs] Ordnerstruktur-Dokumentation hinzugefügt`
  - `[game] Player-Jump-Logik verbessert + Doku aktualisiert`
  - `[scripts] Wiki-Link-Checker optimiert für .venv`
- **Doku-Updates:** Wenn Verhalten oder API ändert → Doku-Dateien _mit im selben Commit_.

## 11) Lernpunkte & Memory
- Erkannte Probleme oder Lösungen dokumentieren:
  - In `/memories/repo/` (nur via Copilot) für Repository-Fakten
  - In `/memories/` (User-Memory) für allgemeine Patterns
  - Beispiel: Python-PATH-Probleme in CI/Local unterschiedlich → In Memory notieren
- **Ziel:** Künftige Sessions erhalten Kontextwissen und vermeiden bekannte Fallstricke.

## Doku-Checkliste (vor Abschluss)
- [ ] Sprachregel eingehalten (Deutsch)
- [ ] Neuer/geänderter Code ausführlich auf Deutsch kommentiert
- [ ] Betroffene Doku aktualisiert
- [ ] Eintrag/Verlinkung in [[docs/GESAMTDOKU]] vorhanden (falls neues Thema)
- [ ] „Verwandte Dokus (Obsidian)" gepflegt
- [ ] Validierungswerkzeuge lokal oder via CI bestätigt (grün)
- [ ] Commit-Message mit [BEREICH] und Doku-Hinweis

## Geltungsbereich
- Gilt für alle neuen KI-Sessions in diesem Repository.
- Technische Einbindung erfolgt über `.github/copilot-instructions.md`.
