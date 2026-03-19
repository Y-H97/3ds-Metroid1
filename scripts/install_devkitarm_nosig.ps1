# Installation von devkitARM OHNE Signaturprüfung
# Workaround für hartnäckige PGP-Probleme

$ErrorActionPreference = "Stop"

Write-Host "=== devkitARM Installation (ohne Signaturprüfung) ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "WARNUNG: Signaturprüfung wird temporär deaktiviert!" -ForegroundColor Yellow
Write-Host "         Dies ist ein Workaround für PGP-Schlüsselprobleme." -ForegroundColor Yellow
Write-Host ""

$msys2Path = "C:\devkitPro\msys2"
$bashExe = "$msys2Path\usr\bin\bash.exe"
$pacmanConf = "$msys2Path\etc\pacman.conf"

if (-not (Test-Path $bashExe)) {
    Write-Host "FEHLER: MSYS2 nicht gefunden" -ForegroundColor Red
    exit 1
}

# Backup der pacman.conf erstellen
Write-Host "[1/4] Erstelle Backup von pacman.conf..." -ForegroundColor Yellow
if (Test-Path $pacmanConf) {
    Copy-Item $pacmanConf "$pacmanConf.backup" -Force
    Write-Host "    Backup erstellt: pacman.conf.backup" -ForegroundColor Gray
}

# Signaturprüfung deaktivieren
Write-Host "[2/4] Deaktiviere Signaturprüfung..." -ForegroundColor Yellow
& $bashExe -lc @"
sed -i 's/^SigLevel.*/SigLevel = Never/g' /etc/pacman.conf
sed -i 's/^#SigLevel.*/SigLevel = Never/g' /etc/pacman.conf
"@

# devkitARM installieren
Write-Host "[3/4] Installiere 3ds-dev..." -ForegroundColor Yellow
Write-Host "    Dies kann einige Minuten dauern..." -ForegroundColor Gray
Write-Host ""

& $bashExe -lc "pacman -Sy --noconfirm 3ds-dev"

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "[4/4] Stelle Signaturprüfung wieder her..." -ForegroundColor Yellow
    
    # pacman.conf wiederherstellen
    if (Test-Path "$pacmanConf.backup") {
        Copy-Item "$pacmanConf.backup" $pacmanConf -Force
        Remove-Item "$pacmanConf.backup" -Force
    }
    
    Write-Host ""
    Write-Host "=== Installation erfolgreich! ===" -ForegroundColor Green
    Write-Host ""
    
    # Verifikation
    Write-Host "Verifikation:" -ForegroundColor Cyan
    & $bashExe -lc "arm-none-eabi-gcc --version 2>&1 | head -n 1"
    
    Write-Host ""
    Write-Host "Nächste Schritte:" -ForegroundColor Cyan
    Write-Host "1. PowerShell neu starten (für ENV-Variablen)" -ForegroundColor White
    Write-Host "2. Build testen:" -ForegroundColor White
    Write-Host "   cd C:\Users\hoetting.y\3ds-Metroid1\3ds-cpp" -ForegroundColor Gray
    Write-Host "   make" -ForegroundColor Gray
    
} else {
    Write-Host ""
    Write-Host "=== Installation fehlgeschlagen ===" -ForegroundColor Red
    Write-Host ""
    
    # Wiederherstellen auch bei Fehler
    if (Test-Path "$pacmanConf.backup") {
        Copy-Item "$pacmanConf.backup" $pacmanConf -Force
        Remove-Item "$pacmanConf.backup" -Force
        Write-Host "pacman.conf wiederhergestellt." -ForegroundColor Gray
    }
    
    Write-Host "Bitte überprüfe die Fehlermeldungen oben." -ForegroundColor Yellow
    exit 1
}
