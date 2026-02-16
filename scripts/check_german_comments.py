#!/usr/bin/env python3
"""Prüft, ob geänderte Quell‑/Skriptdateien deutsche Kommentare enthalten.

Zweck:
- Wird von `pre-commit` und CI verwendet, um sicherzustellen, dass neuer/Geänderter Code
  mindestens einen deutschen Kommentar (Umlaute oder typische deutsche Wörter)
  enthält — laut Repository‑Regel "Deutsche Kommentare verpflichtend".

Verhalten:
- Wenn Dateinamen als Argumente übergeben werden, werden diese geprüft.
- Wenn keine Argumente übergeben werden, versucht das Skript geänderte Dateien
  gegenüber `origin/main` zu ermitteln (CI/PR‑Usecase). Falls das fehlschlägt,
  werden alle trackten Dateien durchsucht.

Exit‑Code: 0 = OK, 1 = Mindestens eine Datei verletzt die Regel.
"""
import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

# Repository root (Script-Ort verwenden, nicht CWD — repariert UNC/CMD‑Problem)
REPO_ROOT = Path(__file__).resolve().parent.parent

# Dateiendungen, für die die Kommentarpflicht gilt
CHECK_EXTENSIONS = {".c", ".cpp", ".h", ".hpp", ".lua", ".py"}
# Ausschluss‑Pfadmuster (Build / generierte Dateien etc.)
# Beachte: wir normalisieren Pfade auf '/' bevor wir prüfen, daher reicht '/build/'.
EXCLUDE_PATH_PARTS = ("/build/", "/romfs/", "third_party", "vendor", "node_modules")
# Einfache Heuristik für "deutsche" Kommentare: Umlaute/ß oder deutsche Stopwörter (inkl. einige gebräuchliche Konjugationen)
GERMAN_RE = re.compile(r"[äöüß]|\b(die|der|und|für|mit|nicht|ist|kein|eine|ein|auch|sind|werden|wird|vom|falls|wenn|bei|durch|siehe)\b", re.IGNORECASE)

CPP_COMMENT_RE = re.compile(r"//.*|/\*[\s\S]*?\*/")
LUA_COMMENT_RE = re.compile(r"--.*|--\[\[[\s\S]*?\]\]")
# Für Python: '#...' sowie dreifach-quoted Docstrings; benutze non-capturing group damit
# re.findall für beide Alternativen den kompletten Match zurückliefert.
PY_COMMENT_RE = re.compile(r"#.*|(?:'''[\s\S]*?'''|\"\"\"[\s\S]*?\"\"\")")


def git_changed_files():
    """Versucht geänderte Dateien gegenüber origin/main zu ermitteln (arbeitet im Repo‑Root)."""
    try:
        subprocess.run(["git", "fetch", "origin", "main", "--depth=1"], check=False, stdout=subprocess.DEVNULL, cwd=REPO_ROOT)
        out = subprocess.check_output(["git", "diff", "--name-only", "origin/main...HEAD"], cwd=REPO_ROOT).decode("utf-8")
        files = [l.strip() for l in out.splitlines() if l.strip()]
        return files
    except Exception:
        return []


def git_all_files():
    try:
        out = subprocess.check_output(["git", "ls-files"], cwd=REPO_ROOT).decode("utf-8")
        return [l.strip() for l in out.splitlines() if l.strip()]
    except Exception:
        return []


def extract_comments(text: str, ext: str) -> str:
    """Extrahiert kommentierten Text (als zusammengefügten String) anhand der Dateiendung."""
    if ext in (".c", ".cpp", ".h", ".hpp"):
        matches = CPP_COMMENT_RE.findall(text)
    elif ext == ".lua":
        matches = LUA_COMMENT_RE.findall(text)
    elif ext == ".py":
        matches = PY_COMMENT_RE.findall(text)
    else:
        matches = []
    # matches können Tupel (bei Gruppen) oder Strings sein; unify
    normalized = []
    for m in matches:
        if isinstance(m, tuple):
            normalized.append(" ".join(m))
        else:
            normalized.append(m)
    return "\n".join(normalized)


def should_check(path: Path) -> bool:
    s = str(path).replace('\\\\', '/')
    if any(p in s for p in EXCLUDE_PATH_PARTS):
        return False
    return path.suffix.lower() in CHECK_EXTENSIONS


def looks_german(text: str) -> bool:
    return bool(GERMAN_RE.search(text))


def check_file(path: Path) -> bool:
    try:
        content = path.read_text(encoding="utf-8")
    except Exception:
        # Bei binären/ungeeigneten Dateien gelten sie nicht als zu prüfen
        return True
    comments = extract_comments(content, path.suffix.lower())
    if not comments.strip():
        return False

    # Ausnahme: LevelEditor/level/*.lua sind Raum‑Daten‑Dateien — dort genügt ein Kommentar.
    # Verwende posix‑Pfad für portable Vergleiche (Windows hat backslashes).
    s = path.as_posix()
    if s.startswith('LevelEditor/level/') and path.suffix.lower() == '.lua':
        return True

    return looks_german(comments)


def main():
    parser = argparse.ArgumentParser(description="Prüft geänderte Dateien auf deutsche Kommentare")
    parser.add_argument("files", nargs="*", help="Dateien, die geprüft werden sollen (falls leer: git-diff oder alle Dateien)")
    args = parser.parse_args()

    files = args.files or git_changed_files()

    if not files:
        # Fallback: überprüfe alle getrackten Dateien (nicht ideal, aber nützlich lokal)
        files = git_all_files()

    # Normalisiere angegebenen Dateipfade zu absoluten Paths (Repository‑Root als Basis).
    normalized_paths = []
    for f in files:
        if not f:
            continue
        p = Path(f)
        if not p.is_absolute():
            p = (REPO_ROOT / p).resolve()
        normalized_paths.append(p)

    # Filtere nur relevante Dateitypen
    paths_to_check = [p for p in normalized_paths if should_check(p)]


    if not paths_to_check:
        print("Keine relevanten Quelldateien zum Prüfen gefunden — Prüfung übersprungen.")
        return 0

    failed = []
    for p in sorted(set(paths_to_check)):
        if not p.exists():
            # Datei evtl. gelöscht/umbenannt — ignoriere
            continue
        ok = check_file(p)
        if not ok:
            try:
                failed.append(str(p.relative_to(REPO_ROOT)))
            except Exception:
                failed.append(str(p))

    if failed:
        print("FEHLER: Die folgenden Dateien enthalten keine deutschen Kommentare (erforderlich):")
        for f in failed:
            print(f"  - {f}")
        print("\nHinweis: Die Regel gilt für neuen/geänderten Quellcode. Füge bitte einen kurzen deutschen Kommentar (z. B. Funktionserklärung) hinzu.")
        return 1

    print("OK: Alle geprüften Dateien enthalten deutsche Kommentare.")
    return 0


if __name__ == '__main__':
    sys.exit(main())
