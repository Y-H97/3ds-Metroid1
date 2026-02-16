# PowerShell wrapper — setzt Arbeitsverzeichnis auf Skriptordner und startet das Python‑Skript.
# Nutzen: funktioniert sauber mit UNC‑Pfaden (PowerShell unterstützt diese).
Set-Location -LiteralPath $PSScriptRoot
if (Test-Path -Path "./check_german_comments.py") {
    & python ./check_german_comments.py @Args
    exit $LASTEXITCODE
} else {
    Write-Error "check_german_comments.py nicht gefunden in $PSScriptRoot"
    exit 2
}