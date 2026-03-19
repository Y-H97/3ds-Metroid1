# Changelog und Versionierung

Dieses Dokument beschreibt das Versionierungs- und Changelog-System des Projekts.

## Übersicht

Das Projekt nutzt ein **zweistufiges Changelog-System**:
- **Hauptdatei** [`CHANGELOG.md`](../../CHANGELOG.md) im Repository-Root (kompakte Übersicht)
- **Detaildateien** unter `changelog/` (ausführliche Beschreibung pro Version)

Vorteile:
- Schneller Überblick aller Versionen im Root
- Detaillierte Änderungshistorie bleibt wartbar und übersichtlich
- Separation of Concerns: Übersicht vs. Details

## Versionierungsschema

Das Projekt folgt **Semantic Versioning** (SemVer 2.0.0):

```
v MAJOR . MINOR . PATCH
  0     . 2     . 2
```

### Regeln

| Version | Wann erhöhen? | Beispiel |
|---------|---------------|----------|
| **MAJOR** (0.x.x) | Breaking Changes, grundlegende Architekturänderungen | API-Änderung, die altes Savegame unbrauchbar macht |
| **MINOR** (x.1.x) | Neue Features, größere Funktionalität hinzugefügt | Neues Item-System, neuer Level-Editor-Modus |
| **PATCH** (x.x.1) | Bugfixes, kleine Verbesserungen ohne neue Features | Absturzfix, UI-Anpassung, Performance-Verbesserung |

**Aktueller Stand**: v0.2.2 (noch in Pre-Release-Phase, daher MAJOR = 0)

### Release-Workflow

#### 1. Entscheiden: Welche Versionsnummer?

Basierend auf den Änderungen seit der letzten Version:
- **Bugfixes only?** → PATCH erhöhen (z. B. 0.2.2 → 0.2.3)
- **Neue Features?** → MINOR erhöhen, PATCH auf 0 (z. B. 0.2.2 → 0.3.0)
- **Breaking Changes?** → MAJOR erhöhen, MINOR+PATCH auf 0 (z. B. 0.2.2 → 1.0.0)

#### 2. Changelog-Datei erstellen

Neue Datei unter `changelog/` anlegen:

```bash
# Beispiel für v0.3.0
changelog/v0.3.0.md
```

**Format-Vorlage:**

```markdown
# v0.3.0 — YYYY-MM-DD

Kurzbeschreibung des Releases (1-2 Sätze, was war der Fokus?).

## Hinzugefügt
- Neue Feature X implementiert.
- Neue Funktion Y im Editor verfügbar.

## Geändert
- Bestehende Funktion Z verbessert.
- Performance-Optimierung in Modul A.

## Behoben
- Abstürze beim Laden von Level B behoben.
- UI-Bug in Menü C korrigiert.

---

`Changelog` generiert automatisch aus den Commits seit vX.Y.Z-Tag.
```

**Kategorien** (in dieser Reihenfolge verwenden):
1. **Hinzugefügt** – Neue Features, Funktionalität
2. **Geändert** – Änderungen an bestehender Funktionalität
3. **Veraltet** – Features, die bald entfernt werden (optional)
4. **Entfernt** – Gelöschte Features (optional)
5. **Behoben** – Bugfixes
6. **Sicherheit** – Sicherheitsrelevante Änderungen (optional)

Nicht verwendete Kategorien weglassen.

#### 3. CHANGELOG.md aktualisieren

Die Hauptdatei [`CHANGELOG.md`](../../CHANGELOG.md) am Anfang (nach "Unreleased") erweitern:

```markdown
## [v0.3.0] - YYYY-MM-DD
- Siehe `changelog/v0.3.0.md` für Details.
```

#### 4. Git-Tag erstellen

Nach Commit der Änderungen Git-Tag setzen:

```powershell
# Version committen
git add CHANGELOG.md changelog/v0.3.0.md
git commit -m "Release v0.3.0"

# Tag erstellen
git tag -a v0.3.0 -m "Release v0.3.0"

# Tag pushen (wenn bereit)
git push origin v0.3.0
```

#### 5. Veröffentlichung (optional)

Bei größeren Releases:
- GitHub Release erstellen (falls GitHub verwendet wird)
- Binärdateien (.3dsx) als Release-Assets anhängen
- Changelog aus `changelog/v0.X.Y.md` in Release-Notes kopieren

## Best Practices

### Changelog schreiben

✅ **GUT:**
```markdown
## Hinzugefügt
- Vollständiges Item-/Inventarsystem inkl. Doppelsprung-Upgrade.
- Touch-/Mouse-Unterstützung im Simulator und auf dem 3DS.
```

❌ **SCHLECHT:**
```markdown
## Hinzugefügt
- Verschiedenes
- Fixes
```

**Regeln:**
1. **Beschreibend**: Was wurde geändert? Welchen Nutzen hat es?
2. **Konkret**: Nicht "UI verbessert", sondern "Zurück-Button in allen Editor-Ansichten hinzugefügt"
3. **Nutzerorientiert**: Aus Sicht der Anwender (Level-Designer, Entwickler, Spieler)
4. **Deutsch**: Projektstandard ist deutsche Dokumentation
5. **Gruppieren**: Zusammenhängende Änderungen zusammenfassen

### Unreleased-Bereich

Zwischen Releases Änderungen im "Unreleased"-Bereich sammeln:

```markdown
## Unreleased
- Arbeitsstand / kleinere Änderungen (nicht veröffentlicht).
- Neue Funktion X (noch in Arbeit).
```

Bei Release:
1. "Unreleased"-Einträge in neue Versionsdatei übernehmen
2. "Unreleased"-Bereich im CHANGELOG.md leeren

### Versionsnummern-Historie

| Version | Datum | Hauptänderungen |
|---------|-------|-----------------|
| v0.2.2 | 2026-03-02 | Item-/Inventarsystem, Touch-Support |
| v0.2.1 | 2026-02-23 | UX-Verbesserungen im Level-Editor |
| v0.2.0 | 2026-02-23 | Hauptmenü und UI-Funktionen |
| v0.1.0 | 2026-02-16 | Dokumentation, CI/CD, Pre-commit Hooks |

## Technische Details

### Dateistruktur

```
3ds-Metroid1/
├── CHANGELOG.md              # Hauptübersicht (kompakt)
└── changelog/                # Detaillierte Versionshistorie
    ├── v0.1.0.md
    ├── v0.2.0.md
    ├── v0.2.1.md
    └── v0.2.2.md
```

### Automatisierung

**Aktuelle Automatisierung:**
- Git-Tags markieren Releases
- Changelog wird manuell erstellt (aus Commit-Historie)

**Mögliche zukünftige Automatisierung:**
- Skript zum Generieren von Changelog-Vorlagen aus Commits
- Automatische Versionsnummern-Erhöhung per Git-Hook
- CI-Job zur Validierung des Changelog-Formats

## Verwandte Dokus (Obsidian)

- [[docs/tooling/README|Tooling-Übersicht]] – Build-Skripte und Prozesse
- [[docs/AI_REGELN|AI-Regeln]] – Projektregeln und Standards
- [[docs/GESAMTDOKU|Gesamtdokumentation]] – Zentraler Doku-Hub
- [[docs/workspace/README|Workspace-Struktur]] – Repository-Aufbau

---

**Letzte Aktualisierung:** 2026-03-09  
**Aktuelle Version:** v0.2.2
