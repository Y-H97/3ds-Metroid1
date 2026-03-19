# devkitARM Installation (Fehlerbehebung)

## Problem: PGP-Schlüssel-Fehler bei pacman

Wenn beim Installieren von `3ds-dev` Fehler wie diese auftreten:
```
error: msys: key "5F944B027F7FE2091985AA2EFA11531AA0AA7F57" is unknown
:: Import PGP key 5F944B027F7FE2091985AA2EFA11531AA0AA7F57? [Y/n]
```

---

## Lösung 1: Schlüsselbund aktualisieren (empfohlen)

In der MSYS2-Shell (`C:\devkitPro\msys2\msys2_shell.cmd`):

```bash
# Schlüsselbund neu initialisieren
pacman-key --init

# devkitPro-Schlüssel importieren
pacman-key --recv-keys --keyserver keyserver.ubuntu.com 5F944B027F7FE2091985AA2EFA11531AA0AA7F57

# Schlüssel signieren
pacman-key --lsign-key 5F944B027F7FE2091985AA2EFA11531AA0AA7F57

# Jetzt Installation versuchen
pacman -Sy
pacman -S 3ds-dev
```

---

## Lösung 2: Komplette Schlüssel-Neuerstellung

Falls Lösung 1 nicht funktioniert:

```bash
# Alte Schlüssel entfernen
rm -rf /etc/pacman.d/gnupg

# Neu initialisieren
pacman-key --init
pacman-key --populate msys2
pacman-key --populate archlinux

# devkitPro-Schlüssel manuell hinzufügen
pacman-key --recv-keys --keyserver keyserver.ubuntu.com 5F944B027F7FE2091985AA2EFA11531AA0AA7F57
pacman-key --lsign-key 5F944B027F7FE2091985AA2EFA11531AA0AA7F57

# Installation
pacman -Sy 3ds-dev
```

---

## Lösung 3: Signatur-Prüfung temporär deaktivieren (nur als letzter Ausweg)

**⚠️ Warnung:** Unsicherer Workaround, nur verwenden wenn alle anderen Methoden fehlschlagen.

1. Konfigurationsdatei bearbeiten:
   ```bash
   nano /etc/pacman.conf
   ```

2. Zeile suchen:
   ```
   SigLevel    = Required DatabaseOptional
   ```
   
3. Ändern zu:
   ```
   SigLevel    = Never
   ```

4. Speichern (`Ctrl+O`, `Enter`, `Ctrl+X`) und Installation:
   ```bash
   pacman -Sy 3ds-dev
   ```

5. **Wichtig:** Nach erfolgreicher Installation wieder rückgängig machen:
   ```bash
   nano /etc/pacman.conf
   # Zurückändern zu: SigLevel = Required DatabaseOptional
   ```

---

## Lösung 4: Alternative Keyserver

Falls der Ubuntu-Keyserver nicht erreichbar ist:

```bash
# Alternative Keyserver ausprobieren:
pacman-key --recv-keys --keyserver keys.openpgp.org 5F944B027F7FE2091985AA2EFA11531AA0AA7F57

# oder:
pacman-key --recv-keys --keyserver hkps://keyserver.ubuntu.com 5F944B027F7FE2091985AA2EFA11531AA0AA7F57

# oder:
pacman-key --recv-keys --keyserver pgp.mit.edu 5F944B027F7FE2091985AA2EFA11531AA0AA7F57
```

---

## Lösung 5: devkitPro-Updater verwenden (Windows)

Falls MSYS2 pacman weiterhin Probleme macht:

1. **devkitPro Updater** herunterladen:
   - https://github.com/devkitPro/installer/releases
   - `devkitProUpdater.exe` ausführen

2. **In der Updater-GUI:**
   - `Nintendo 3DS` auswählen
   - Installieren lassen (benötigt Internet)

3. **Vorteil:** Umgeht pacman-Schlüsselprobleme komplett

---

## Verifikation der Installation

Nach erfolgreicher Installation prüfen:

```bash
# devkitARM-Version anzeigen
arm-none-eabi-gcc --version

# Pfad prüfen
ls -la /opt/devkitpro/devkitARM
```

In PowerShell:

```powershell
# Umgebungsvariablen prüfen
$env:DEVKITPRO   # sollte /opt/devkitpro zeigen
$env:DEVKITARM   # sollte /opt/devkitpro/devkitARM zeigen

# Compiler-Verfügbarkeit testen
& "C:\devkitPro\msys2\opt\devkitpro\devkitARM\bin\arm-none-eabi-gcc.exe" --version
```

---

## Verwandte Dokus (Obsidian)
- [[docs/game/DEPLOYMENT_3DS|Deployment auf Nintendo 3DS]]
- [[docs/game/SETUP|Game Setup]]
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
