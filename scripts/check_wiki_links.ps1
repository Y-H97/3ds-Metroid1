# PowerShell wrapper — setzt Arbeitsverzeichnis auf Skriptordner und startet das Python‑Skript.
Set-Location -LiteralPath $PSScriptRoot
if (Test-Path -Path "./check_wiki_links.py") {
    & python ./check_wiki_links.py @Args
    exit $LASTEXITCODE
} else {
    Write-Error "check_wiki_links.py nicht gefunden in $PSScriptRoot"
    exit 2
}