# PowerShell wrapper — setzt Arbeitsverzeichnis auf Skriptordner und startet das Python‑Skript.
# Versucht zuerst, das Workspace-Python zu nutzen (falls vorhanden), sonst das System-Python.

# Der Ordner dieses Skripts ist scripts/
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
# Gehe eine Ebene rauf zum Workspace-Root (3ds-Metroid1/)
$WorkspaceRoot = Split-Path -Parent $ScriptDir
$VEnvPython = Join-Path -Path $WorkspaceRoot -ChildPath ".venv\Scripts\python.exe"
$SystemPython = "python"

Set-Location -LiteralPath $ScriptDir

# Prüfe, ob das Skript existiert
if (-not (Test-Path -Path "./check_wiki_links.py")) {
    Write-Error "check_wiki_links.py nicht gefunden in $ScriptDir"
    exit 2
}

# Weise Python-Interpreter zu: Workspace zuerst, sonst System
if (Test-Path -LiteralPath $VEnvPython) {
    Write-Host "[INFO] Nutze Workspace-Python: $VEnvPython" -ForegroundColor Cyan
    & $VEnvPython ./check_wiki_links.py @Args
} else {
    Write-Host "[WARN] Workspace-Python nicht gefunden auf $VEnvPython; nutze System-Python" -ForegroundColor Yellow
    & $SystemPython ./check_wiki_links.py @Args
}

exit $LASTEXITCODE