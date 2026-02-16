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

## Doku-Checkliste (vor Abschluss)
- [ ] Sprachregel eingehalten (Deutsch)
- [ ] Neuer/geänderter Code ausführlich auf Deutsch kommentiert
- [ ] Betroffene Doku aktualisiert
- [ ] Eintrag/Verlinkung in [[docs/GESAMTDOKU]] vorhanden (falls neues Thema)
- [ ] „Verwandte Dokus (Obsidian)“ gepflegt
- [ ] Kurzvalidierung durchgeführt oder begründet ausgelassen

## Geltungsbereich
- Gilt für alle neuen KI-Sessions in diesem Repository.
- Technische Einbindung erfolgt über `.github/copilot-instructions.md`.
