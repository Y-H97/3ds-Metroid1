# devkitARM Installation für Windows
# Führt das Bash-Installationsskript in MSYS2 aus

$ErrorActionPreference = "Stop"

Write-Host "=== devkitARM Installation ===" -ForegroundColor Cyan
Write-Host ""

# Pfade prüfen
$msys2Path = "C:\devkitPro\msys2"
$bashExe = "$msys2Path\usr\bin\bash.exe"
$scriptPath = "$PSScriptRoot\install_devkitarm.sh"

if (-not (Test-Path $bashExe)) {
    Write-Host "FEHLER: MSYS2 nicht gefunden unter: $msys2Path" -ForegroundColor Red
    Write-Host "Bitte installiere devkitPro von: https://github.com/devkitPro/installer/releases" -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $scriptPath)) {
    Write-Host "FEHLER: Installationsskript nicht gefunden: $scriptPath" -ForegroundColor Red
    exit 1
}

# Konvertiere Windows-Pfad zu MSYS2-Pfad
$scriptPathUnix = $scriptPath -replace '\\', '/' -replace 'C:', '/c'

Write-Host "Starte Installation in MSYS2..." -ForegroundColor Yellow
Write-Host "Dies kann einige Minuten dauern..." -ForegroundColor Gray
Write-Host ""

# Führe das Installationsskript aus
& $bashExe -lc "bash '$scriptPathUnix'"

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "=== Installation erfolgreich! ===" -ForegroundColor Green
    Write-Host ""
    Write-Host "Nächste Schritte:" -ForegroundColor Cyan
    Write-Host "1. Diese PowerShell-Sitzung schließen und neu öffnen" -ForegroundColor White
    Write-Host "2. Build testen:" -ForegroundColor White
    Write-Host "   cd C:\Users\hoetting.y\3ds-Metroid1\3ds-cpp" -ForegroundColor Gray
    Write-Host "   make" -ForegroundColor Gray
} else {
    Write-Host ""
    Write-Host "=== Installation fehlgeschlagen ===" -ForegroundColor Red
    Write-Host ""
    Write-Host "Alternative Lösungen:" -ForegroundColor Yellow
    Write-Host "1. devkitPro Updater GUI verwenden:" -ForegroundColor White
    Write-Host "   https://github.com/devkitPro/installer/releases" -ForegroundColor Gray
    Write-Host "2. Dokumentation lesen:" -ForegroundColor White
    Write-Host "   docs\game\DEVKITARM_INSTALLATION.md" -ForegroundColor Gray
    exit 1
}
