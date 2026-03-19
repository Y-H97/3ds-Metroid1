# Manuelle Installation von devkitARM - EINFACHSTE METHODE  

## Option 1: MSYS2-Shell (5 Minuten, empfohlen)

### Schritt 1: MSYS2-Shell öffnen
```batch
C:\devkitPro\msys2\msys2_shell.cmd
```

### Schritt 2: In der MSYS2-Shell eingeben
```bash
# Lock entfernen (falls vorhanden)
rm -f /var/lib/pacman/db.lck

# Installation starten
pacman -Sy devkitARM libctru citro3d citro2d 3dstools 3dslink tex3ds picasso

# Bei Fragen einfach ENTER drücken (= "Yes" / "All")
```

### Schritt 3: Warten (3-5 Minuten Download)
Der Download ist ca. 100 MB und zeigt einen Fortschrittsbalken.

### Schritt 4: Verifizieren
```bash
arm-none-eabi-gcc --version
```

Wenn du die Version siehst → **Installation erfolgreich!**

### Schritt 5: PowerShell neu starten und Build testen
```powershell
cd C:\Users\hoetting.y\3ds-Metroid1\3ds-cpp
make
```

---

## Option 2: Automatisches Skript in MSYS2

Falls du das Skript bevorzugst:

```bash
# In MSYS2-Shell:
bash /c/Users/hoetting.y/3ds-Metroid1/scripts/install_devkitarm_unattended.sh
```

---

## Option 3: Einzelne Pakete manuell in PowerShell

Falls MSYS2-Shell nicht funktioniert, installiere die Pakete einzeln:

```powershell
# Signat urprüfung deaktivieren
$conf = "C:\devkitPro\msys2\etc\pacman.conf"
(Get-Content $conf) -replace '^SigLevel\s*=.*', 'SigLevel = Never' | Set-Content $conf

# Lock entfernen
Remove-Item "C:\devkitPro\msys2\var\lib\pacman\db.lck" -Force -ErrorAction SilentlyContinue

# Datenbank aktualisieren
C:\devkitPro\msys2\usr\bin\bash.exe -lc "pacman -Sy --noconfirm"

# Hauptpaket installieren (dauert 5-10 Minuten)
C:\devkitPro\msys2\usr\bin\bash.exe -lc "pacman -S --needed --noconfirm devkitARM"

# 3DS-Bibliotheken installieren
C:\devkitPro\msys2\usr\bin\bash.exe -lc "pacman -S --needed --noconfirm libctru citro3d citro2d"

# Tools installieren
C:\devkitPro\msys2\usr\bin\bash.exe -lc "pacman -S --needed --noconfirm 3dstools 3dslink tex3ds picasso"
```

---

## Warum schlagen die automatischen Skripte fehl?

Der Download ist ~100 MB und dauert 3-5 Minuten. Die PowerShell-Automatisierung wird unterbrochen, weil:
1. pacman manchmal auf Eingabe wartet (trotz `--noconfirm`)
2. Lange Downloads ohne neue Ausgabe als "idle" interpretiert werden

**Lösung:** Manuelle Installation in MSYS2-Shell (Option 1) ist der zuverlässigste Weg.

---

## Nach der Installation

Überprüfe, dass devkitARM korrekt installiert ist:

```powershell
C:\devkitPro\msys2\usr\bin\bash.exe -lc "ls -la /opt/devkitpro/devkitARM"
C:\devkitPro\msys2\usr\bin\bash.exe -lc "arm-none-eabi-gcc --version"
```

Dann **PowerShell neu starten** und Build testen:

```powershell
cd C:\Users\hoetting.y\3ds-Metroid1\3ds-cpp
make
```
