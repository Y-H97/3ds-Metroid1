# Manuelle Installation von devkitARM
# BITTE IN EINEM NEUEN POWERSHELL-FENSTER AUSFÜHREN

## Schritt 1: Alle Prozesse beenden
```powershell
Get-Process | Where-Object { $_.Name -match 'pacman|gpg|bash' } | Stop-Process -Force -ErrorAction SilentlyContinue
```

## Schritt 2: pacman.conf sichern
```powershell
Copy-Item "C:\devkitPro\msys2\etc\pacman.conf" "C:\devkitPro\msys2\etc\pacman.conf.backup"
```

## Schritt 3: Signaturprüfung deaktivieren (temp)
```powershell
$conf = "C:\devkitPro\msys2\etc\pacman.conf"
(Get-Content $conf) -replace '^SigLevel\s*=.*', 'SigLevel = Never' -replace '^#SigLevel.*', '#SigLevel = Never' | Set-Content $conf
```

## Schritt 4: devkitARM installieren
```powershell
C:\devkitPro\msys2\usr\bin\bash.exe -lc "pacman -Sy --noconfirm 3ds-dev"
```

## Schritt 5: Prüfen ob Installation erfolgreich war
```powershell
C:\devkitPro\msys2\usr\bin\bash.exe -lc "arm-none-eabi-gcc --version"
```

## Schritt 6: pacman.conf wiederherstellen
```powershell
Copy-Item "C:\devkitPro\msys2\etc\pacman.conf.backup" "C:\devkitPro\msys2\etc\pacman.conf" -Force
```

## Schritt 7: Build testen
```powershell
cd C:\Users\hoetting.y\3ds-Metroid1\3ds-cpp
make
```

---

## ODER: Alles auf einmal ausführen

Kopiere diesen gesamten Block in ein **neues PowerShell-Fenster**:

```powershell
# Stoppe alte Prozesse
Get-Process | Where-Object { $_.Name -match 'pacman|gpg|bash' } | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2

# Backup erstellen
Copy-Item "C:\devkitPro\msys2\etc\pacman.conf" "C:\devkitPro\msys2\etc\pacman.conf.backup" -Force

# Signaturprüfung deaktivieren
$conf = "C:\devkitPro\msys2\etc\pacman.conf"
(Get-Content $conf) -replace '^SigLevel\s*=.*', 'SigLevel = Never' | Set-Content $conf

Write-Host "=== Installiere 3ds-dev (kann 5-10 Minuten dauern) ===" -ForegroundColor Cyan

# Installation
C:\devkitPro\msys2\usr\bin\bash.exe -lc "pacman -Sy --noconfirm 3ds-dev"

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "=== Installation erfolgreich! ===" -ForegroundColor Green
    
    # Verifikation
    Write-Host ""
    Write-Host "Compiler-Version:" -ForegroundColor Cyan
    C:\devkitPro\msys2\usr\bin\bash.exe -lc "arm-none-eabi-gcc --version | head -n 1"
    
    # Wiederherstellen
    Copy-Item "C:\devkitPro\msys2\etc\pacman.conf.backup" "C:\devkitPro\msys2\etc\pacman.conf" -Force
    
    Write-Host ""
    Write-Host "Nächster Schritt: Build testen" -ForegroundColor Cyan
    Write-Host "  cd C:\Users\hoetting.y\3ds-Metroid1\3ds-cpp" -ForegroundColor Gray
    Write-Host "  make" -ForegroundColor Gray
} else {
    Write-Host ""
    Write-Host "=== Installation fehlgeschlagen ===" -ForegroundColor Red
    # Wiederherstellen auch bei Fehler
    Copy-Item "C:\devkitPro\msys2\etc\pacman.conf.backup" "C:\devkitPro\msys2\etc\pacman.conf" -Force
}
```
