# Komplette Neuinitialisierung der pacman-Schlüsseldatenbank
# Behebt hartnäckige PGP-Schlüsselprobleme

$ErrorActionPreference = "Stop"

Write-Host "=== pacman Schlüsseldatenbank Neuaufbau ===" -ForegroundColor Cyan
Write-Host ""

$msys2Path = "C:\devkitPro\msys2"
$bashExe = "$msys2Path\usr\bin\bash.exe"

if (-not (Test-Path $bashExe)) {
    Write-Host "FEHLER: MSYS2 nicht gefunden unter: $msys2Path" -ForegroundColor Red
    exit 1
}

Write-Host "[1/5] Stoppe laufende pacman-Prozesse..." -ForegroundColor Yellow
Get-Process | Where-Object { $_.Name -like "*pacman*" -or $_.Name -like "*gpg*" } | Stop-Process -Force -ErrorAction SilentlyContinue

Write-Host "[2/5] Lösche alte Schlüsseldatenbank..." -ForegroundColor Yellow
$gnupgPath = "$msys2Path\etc\pacman.d\gnupg"
if (Test-Path $gnupgPath) {
    Remove-Item -Path $gnupgPath -Recurse -Force -ErrorAction SilentlyContinue
    Write-Host "    Alte Datenbank entfernt." -ForegroundColor Gray
}

Write-Host "[3/5] Initialisiere neue Schlüsseldatenbank..." -ForegroundColor Yellow
& $bashExe -lc "pacman-key --init"

Write-Host "[4/5] Importiere Standardschlüssel..." -ForegroundColor Yellow
& $bashExe -lc "pacman-key --populate msys2"

Write-Host "[5/5] Aktualisiere Paketdatenbank..." -ForegroundColor Yellow
& $bashExe -lc "pacman -Sy --noconfirm"

Write-Host ""
Write-Host "=== Schlüsseldatenbank neu aufgebaut ===" -ForegroundColor Green
Write-Host ""
Write-Host "Nächster Schritt: Installation von devkitARM" -ForegroundColor Cyan
Write-Host ""
Write-Host "Option 1: Automatisch (empfohlen)" -ForegroundColor White
Write-Host "  .\scripts\install_devkitarm.ps1" -ForegroundColor Gray
Write-Host ""
Write-Host "Option 2: Manuell in MSYS2-Shell" -ForegroundColor White
Write-Host "  bash: pacman -S 3ds-dev" -ForegroundColor Gray
Write-Host ""
Write-Host "Option 3: devkitPro Updater GUI erneut versuchen" -ForegroundColor White
Write-Host ""
