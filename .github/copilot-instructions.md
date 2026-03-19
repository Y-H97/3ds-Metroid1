# Copilot-Projektregeln

Diese Anweisungen gelten für dieses Repository und sollen in neuen Copilot-Sessions automatisch berücksichtigt werden.

## Sprache
- Antworte standardmäßig auf Deutsch.
- Verwende Deutsch auch für Code-Kommentare und Dokumentation, sofern nicht explizit anders gewünscht.
- Neuer oder geänderter Code soll ausführlich und einsteigerfreundlich auf Deutsch kommentiert werden.

## Dokumentation
- Führe die Projektdokumentation aktiv weiter, wenn Änderungen fachlich relevant sind.
- Nutze und pflege die zentrale Doku-Navigation:
  - `docs/GESAMTDOKU.md`
- Neue oder geänderte Doku-Seiten sollen Obsidian-kompatibel querverlinkt sein.

## Verbindliche Detailregeln
- Halte dich an:
  - `[[docs/AI_REGELN|docs/AI_REGELN.md]]` – umfassende Regeln für Sprache, Doku, Python, CI, Commits
  - Insbesondere: Validierungswerkzeuge vor Commit, Commit-Message-Format, externe Abhängigkeiten

## Doku-Linkstil
- Für interne Verweise in Markdown bevorzugt Obsidian-Wiki-Links nutzen, z. B.:
  - `[[docs/GESAMTDOKU]]`
  - `[[docs/game/ARCHITECTURE|Game-Architektur]]`

## Arbeitsweise
- Änderungen minimal und zielgerichtet umsetzen.
- Nach Möglichkeit Änderungen kurz verifizieren (Build/Test/Export) und Ergebnis benennen.
- Tasks aus `.vscode/tasks.json` nutzen (z. B. "Check: Obsidian Wiki‑Links") vor Abschluss.

## Validierung vor PR
- Lokal laufen lassen (oder CI wartet):
  - `python scripts/check_wiki_links.py` (falls Doku geändert)
  - `python scripts/check_german_comments.py` (falls Code geändert)
  - `python LevelEditor/export_maps.py` (falls Level geändert)
- GitHub CI überprüft → PR muss grün sein.

## Python & Environment
- Workspace-Python (`/.venv`) wird von Skripten automatisch bevorzugt.
- Falls manuell nötig: `configure_python_environment` verwenden oder `.venv/Scripts/python.exe` direkt aufrufen.

## Memory & Lernpunkte
- Dokumentiere erkannte Probleme/Lösungen in `/memories/repo/` für künftige Sessions.
