PR‑Titel:
chore(docs/ci): Add doc‑checks, pre‑commit hooks and VSCode tasks

Kurzbeschreibung
Fügt automatische Prüfungen für deutsche Code‑Kommentare und Obsidian‑Wiki‑Links hinzu; erweitert pre‑commit, CI‑Jobs, VSCode‑Tasks und PR‑Template. Zusätzlich: deutsche Kopf‑Kommentare in Level‑Dateien und Hinweise im Einsteiger‑Guide.

Wichtige Änderungen
- Scripts: `scripts/check_german_comments.py`, `scripts/check_wiki_links.py` (+ PowerShell/.bat Wrapper)
- CI: `.github/workflows/ci.yml` (erweitert), `.github/workflows/docs-check.yml` (neu)
- pre-commit: `.pre-commit-config.yaml` (Hook für deutsche Kommentare + Wiki‑Links)
- VSCode: `.vscode/tasks.json`, `settings.json` (PowerShell‑Tasks)
- Docs: `docs/GESAMTDOKU.md`, `docs/workspace/EINSTEIGER_GUIDE.md`, `CHANGELOG.md` + `changelog/v0.1.0.md`
- PR‑Template & Repo‑AI‑Regeln: `.github/PULL_REQUEST_TEMPLATE.md`, `docs/AI_REGELN.md`

Validierung / Testschritte (lokal)
- `python scripts/check_german_comments.py`
- `python scripts/check_wiki_links.py`
- `python LevelEditor/export_maps.py` (Export/JSON)
- `pre-commit run --all-files` (optional)

Checklist (bitte vor Merge)
- [ ] CI‑Checks grün (Docs‑Check, Kommentar‑Check, Level‑Export)
- [ ] Build/Simulator lokal gestartet (optional)
- [ ] Dokumentation geprüft / `CHANGELOG.md` ergänzt
- [ ] Reviewer zugewiesen

Labels: `docs`, `ci`, `chore`
Reviewer‑Vorschlag: @team‑owner oder zuständige Maintainer

Weitere Hinweise
- Neuer/geänderter Code muss auf Deutsch kommentiert sein — wird per pre‑commit/CI geprüft.
- Bei Netzwerk/SSL‑Problemen beim Push: verwende SSH‑Remote oder push von anderem Netzwerk/Device.
