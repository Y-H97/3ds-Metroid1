#!/usr/bin/env python3
"""Prüft Obsidian‑Wiki‑Links und dokumentiert nicht verlinkte Seiten unter `docs/`.

Regeln:
- Broken links: jegliche `[[...]]`-Referenz muss auf eine existierende `.md`-Datei zeigen.
- Orphan docs: jede Datei unter `docs/` muss mindestens einmal per Wiki‑Link referenziert sein
  (Ausnahmen: `docs/GESAMTDOKU`, `docs/README`).

Exit‑Code: 0 = OK, 1 = Fehler (broken links oder orphan docs gefunden)

Dieses Skript wird von CI und pre-commit verwendet.
"""
from pathlib import Path
import re
import sys

ROOT = Path('.').resolve()
DOCS_DIR = Path('docs')
WIKI_RE = re.compile(r"\[\[([^\]|]+)(?:\|[^\]]+)?\]\]")

# Dateien, die absichtlich als Index/Dashboard existieren und nicht als 'Orphan' gelten sollen
EXEMPT_DOCS = {"docs/GESAMTDOKU", "docs/README"}


def discover_md_files():
    return [p for p in ROOT.rglob('*.md')]


def canonical_doc_key(path: Path) -> str:
    """Gibt den kanonischen Schlüssel für eine docs-Datei zurück, z.B. 'docs/workspace/README'."""
    rel = path.relative_to(ROOT).as_posix()
    if rel.startswith('docs/') and rel.endswith('.md'):
        return rel[:-3]
    return rel[:-3] if rel.endswith('.md') else rel


def normalize_target(target: str) -> str:
    """Normalisiert einen Wiki‑Link‑Target zu einem kanonischen Pfad ohne .md.

    Beispiele:
      'docs/game/ARCHITECTURE#Abschnitt' -> 'docs/game/ARCHITECTURE'
      'game/ARCHITECTURE' -> 'docs/game/ARCHITECTURE' (Versuch)
      'docs/GESAMTDOKU' -> 'docs/GESAMTDOKU'
    """
    t = target.strip()
    # entferne Anchor
    if '#' in t:
        t = t.split('#', 1)[0]
    # entferne .md
    if t.endswith('.md'):
        t = t[:-3]
    # normalize leading '/'
    if t.startswith('/'):
        t = t[1:]
    # if it already starts with 'docs/', keep it
    if t.startswith('docs/'):
        return t
    # Versuch: wenn Pfad enthält '/', prüfe beide: als-relativ und unter docs/
    if '/' in t:
        return t
    # Standalone name → vermute docs/<name>
    return f"docs/{t}"


def resolve_candidates(key: str):
    """Gib mögliche Dateipfade für einen kanonischen Key zurück."""
    cands = []
    # key kann bereits 'docs/...' oder ein relativer Pfad sein
    if key.startswith('docs/'):
        p = ROOT / (key + '.md')
        cands.append(p)
    else:
        # versuche als repo-relativer Pfad
        cands.append(ROOT / (key + '.md'))
        cands.append(ROOT / ('docs/' + key + '.md'))
    return cands


def find_wiki_links_in_file(path: Path):
    out = []
    text = path.read_text(encoding='utf-8')
    for i, line in enumerate(text.splitlines(), start=1):
        for m in WIKI_RE.finditer(line):
            out.append((i, m.group(1).strip()))
    return out


def main():
    md_files = discover_md_files()

    # 1) Scan aller Wiki‑Links im Repo
    link_map = {}  # target_key -> list of (source_path, line_no, raw_target)
    for md in md_files:
        for ln, raw in find_wiki_links_in_file(md):
            key = normalize_target(raw)
            link_map.setdefault(key, []).append((md, ln, raw))

    # 2) Prüfe Broken Links (existenz der Datei)
    broken = {}
    for key, sources in sorted(link_map.items()):
        # versuche Kandidaten
        cands = resolve_candidates(key)
        exists = any(p.exists() for p in cands)
        if not exists:
            broken[key] = sources

    # 3) Prüfe Orphan Docs (docs/ Dateien ohne inbound links)
    all_docs = [p for p in md_files if p.is_file() and p.resolve().is_relative_to(ROOT) and p.relative_to(ROOT).as_posix().startswith('docs/')]
    # canonical keys of all docs
    all_doc_keys = {canonical_doc_key(p) for p in all_docs}
    # keys that are referenced by wiki-links (normalize to canonical keys where possible)
    referenced = set()
    for k in link_map.keys():
        # If the key points to docs/..., use it; otherwise try to match both
        if k in all_doc_keys:
            referenced.add(k)
        else:
            alt = f"docs/{k}" if not k.startswith('docs/') else k
            if alt in all_doc_keys:
                referenced.add(alt)

    orphan_docs = sorted([d for d in all_doc_keys if d not in referenced and d not in EXEMPT_DOCS])

    # Ausgabe
    if broken:
        print("FEHLER: Gefundene fehlerhafte Obsidian‑Wiki‑Links:")
        for key, sources in broken.items():
            print(f"- Link-Ziel: '{key}' ist nicht vorhanden. Referenziert in:")
            for src, ln, raw in sources:
                print(f"    {src}:{ln}  (\"{raw}\")")
        print()

    if orphan_docs:
        print("FEHLER: Nicht verlinkte Doku‑Seiten unter 'docs/' (Orphans):")
        for d in orphan_docs:
            print(f"- {d}.md")
        print()

    if not broken and not orphan_docs:
        print("OK: Obsidian-Wiki-Links sind konsistent; keine Broken-Links oder Orphans gefunden.")
        return 0

    print("Hinweis: Bitte aktualisiere die Doku oder die Links. Weitere Informationen in docs/GESAMTDOKU.md")
    return 1


if __name__ == '__main__':
    sys.exit(main())
