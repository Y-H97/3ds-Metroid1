#!/usr/bin/env bash
# devkitARM Installation Script (MSYS2)
# Führe dieses Skript in der MSYS2-Shell aus

set -e  # Bei Fehlern abbrechen

echo "=== devkitARM Installation (Unattended) ==="
echo ""

# Lock entfernen falls vorhanden
rm -f /var/lib/pacman/db.lck

# Datenbank aktualisieren
echo "[1/2] Aktualisiere Paketdatenbank..."
pacman -Sy --noconfirm

# Pakete installieren (ohne Nachfrage)
echo "[2/2] Installiere devkitARM und 3DS-Tools..."
pacman -S --needed --noconfirm \
    devkitARM \
    libctru \
    citro3d \
    citro2d \
    3dstools \
    3dslink \
    tex3ds \
    picasso \
    general-tools \
    devkitarm-rules

echo ""
echo "=== Installation abgeschlossen! ==="
echo ""
echo "Compiler-Version:"
arm-none-eabi-gcc --version | head -n 1

echo ""
echo "Installation erfolgreich!"
