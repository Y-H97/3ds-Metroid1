#!/bin/bash
# devkitARM Installation Script für MSYS2
# Löst PGP-Schlüsselprobleme automatisch

echo "=== devkitARM Installation Script ==="
echo ""

# Schritt 1: Schlüsselbund initialisieren
echo "[1/6] Initialisiere Schlüsselbund..."
pacman-key --init

# Schritt 2: Standardschlüssel populieren
echo "[2/6] Populiere MSYS2-Schlüssel..."
pacman-key --populate msys2

# Schritt 3: devkitPro-Schlüssel importieren
echo "[3/6] Importiere devkitPro-Schlüssel..."

# Erster devkitPro-Schlüssel
pacman-key --recv-keys --keyserver keyserver.ubuntu.com 5F944B027F7FE2091985AA2EFA11531AA0AA7F57 2>/dev/null || \
pacman-key --recv-keys --keyserver keys.openpgp.org 5F944B027F7FE2091985AA2EFA11531AA0AA7F57 2>/dev/null || \
pacman-key --recv-keys --keyserver pgp.mit.edu 5F944B027F7FE2091985AA2EFA11531AA0AA7F57

# Zweiter devkitPro-Schlüssel (dkp-libs)
pacman-key --recv-keys --keyserver keyserver.ubuntu.com BC26F752D25B92CE272E0F44F7FD5492264BB9D0 2>/dev/null || \
pacman-key --recv-keys --keyserver keys.openpgp.org BC26F752D25B92CE272E0F44F7FD5492264BB9D0 2>/dev/null || \
pacman-key --recv-keys --keyserver pgp.mit.edu BC26F752D25B92CE272E0F44F7FD5492264BB9D0

# Schritt 4: Schlüssel lokal signieren
echo "[4/6] Signiere Schlüssel lokal..."
pacman-key --lsign-key 5F944B027F7FE2091985AA2EFA11531AA0AA7F57
pacman-key --lsign-key BC26F752D25B92CE272E0F44F7FD5492264BB9D0

# Schritt 5: Paketdatenbank aktualisieren
echo "[5/6] Aktualisiere Paketdatenbank..."
pacman -Sy

# Schritt 6: 3ds-dev installieren
echo "[6/6] Installiere 3ds-dev..."
pacman -S --noconfirm 3ds-dev

echo ""
echo "=== Installation abgeschlossen! ==="
echo ""
echo "Verifikation:"
arm-none-eabi-gcc --version

echo ""
echo "Installation erfolgreich!"
