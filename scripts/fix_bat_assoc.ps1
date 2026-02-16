# fix_bat_assoc.ps1
# Repariert die Windows-Dateizuordnung für .bat-Dateien (setzt assoc/ftype zurück).
# Anleitung: Rechtsklick -> Als Administrator ausführen (oder mit PowerShell: Run as Admin).

# Prüfen ob Skript als Administrator läuft
$principal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
$isAdmin = $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "Starte mit Administratorrechten neu..."
    Start-Process -FilePath pwsh -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`"" -Verb RunAs
    exit
}

Write-Host "Setze .bat Zuordnung (assoc/ftype)..." -ForegroundColor Cyan
# assoc/ftype sind CMD interne Befehle — daher via cmd.exe ausführen
cmd /c "assoc .bat=batfile"
cmd /c "ftype batfile=\"%1\" %*"

Write-Host "Aktuelle Zuordnung:" -ForegroundColor Green
cmd /c "assoc .bat"
cmd /c "ftype batfile"

Write-Host "Fertig. Bitte VS Code neu starten, falls noch Probleme bestehen." -ForegroundColor Yellow
pause
