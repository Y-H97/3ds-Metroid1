# Deployment auf Nintendo 3DS

## Voraussetzungen

### Hardware
- Nintendo 3DS / 3DS XL / 2DS / New 3DS (jegliches Modell)
- SD-Karte (empfohlen: mindestens 4 GB)
- USB-SD-Kartenleser oder PC mit SD-Kartenschacht

### Software auf dem PC (Build-Umgebung)

**devkitPro mit devkitARM muss installiert sein:**

1. **devkitPro installieren** (falls noch nicht geschehen):
   - Download: https://github.com/devkitPro/installer/releases
   - Windows: `devkitProUpdater.exe` ausführen

2. **devkitARM installieren:**

   **Option A: Automatisches Skript (schnell)**
   ```powershell
   # In PowerShell:
   .\scripts\install_devkitarm.ps1
   ```
   
   **Option B: devkitPro Updater GUI (empfohlen bei Problemen)**
   ```batch
   # Öffnet Download-Seite:
   .\scripts\download_devkitpro_updater.bat
   
   # Dann in der GUI "Nintendo 3DS" auswählen
   ```
   
   **Option C: Manuelle Installation via MSYS2**
   ```bash
   # MSYS2-Shell öffnen:
   C:\devkitPro\msys2\msys2_shell.cmd
   
   # In der Shell:
   pacman -Sy
   pacman -S 3ds-dev
   ```
   
   > **Bei PGP-Schlüssel-Problemen:** Siehe [[docs/game/DEVKITARM_INSTALLATION|devkitARM Installation Troubleshooting]]

3. **Umgebungsvariablen prüfen:**
   ```powershell
   $env:DEVKITPRO   # sollte /opt/devkitpro zeigen
   $env:DEVKITARM   # sollte /opt/devkitpro/devkitARM zeigen
   ```
   
   Falls nicht gesetzt, PowerShell neu starten oder System-Umgebungsvariablen prüfen.

### Software auf dem 3DS
Der 3DS benötigt **Homebrew-Fähigkeit**, um `.3dsx`-Dateien auszuführen:

**Option A: Custom Firmware (empfohlen)**
- Luma3DS oder andere CFW installiert
- Homebrew Launcher verfügbar

**Option B: Homebrew Launcher Access**
- Soundhax, Browserhax oder andere Exploit-Methoden
- Begrenzte Funktionalität (kein vollständiger System-Zugriff)

> **Hinweis:** Die Installation von Custom Firmware liegt außerhalb des Projektumfangs.  
> Anleitung: https://3ds.hacks.guide/

---

## Methode 1: Installation via SD-Karte (Standard)

### Schritt 1: Build erstellen
```batch
cd 3ds-cpp
make
```

**Ausgabe:**
- `3ds-cpp/3ds-cpp.3dsx` (Homebrew-Executable)
- `3ds-cpp/3ds-cpp.smdh` (Icon + Metadaten)

### Schritt 2: SD-Karte vorbereiten
1. 3DS ausschalten
2. SD-Karte entnehmen und mit PC verbinden
3. Ordner `/3ds/` auf der SD-Karte öffnen (bei Bedarf anlegen)

### Schritt 3: Dateien kopieren
```
SD-Karte/
├── 3ds/
│   └── metroid/              (Ordner neu anlegen)
│       ├── 3ds-cpp.3dsx
│       └── 3ds-cpp.smdh
```

**Optional:** Projektordner umbenennen (z. B. `metroid` → `Metroidvania`)

### Schritt 4: SD-Karte zurück in 3DS
1. SD-Karte sicher entfernen
2. In 3DS einlegen und einschalten

### Schritt 5: Spiel starten
1. **Homebrew Launcher** öffnen
   - Bei CFW: über Home-Menü oder Rosalina-Menü (`L+Down+Select`)
   - Bei Exploits: je nach Methode (z. B. Sound-App)
2. In der Homebrew-Liste zu **„3ds-cpp"** navigieren
3. Mit `A` starten

---

## Methode 2: Installation via Netzwerk (3dslink)

Für schnelle Test-Deployments ohne SD-Karten-Wechsel.

### Voraussetzungen
- 3DS und PC im **gleichen WLAN**
- `3dslink` installiert (über devkitPro Pacman)
- Homebrew Launcher auf dem 3DS geöffnet

### Installation von 3dslink (einmalig)
```bash
# In MSYS2/devkitPro-Shell:
pacman -S 3dslink
```

### Deployment
```bash
cd 3ds-cpp
make
3dslink 3ds-cpp.3dsx
```

**Ablauf:**
1. `3dslink` sucht automatisch nach 3DS im Netzwerk
2. Überträgt `.3dsx` direkt in den RAM des 3DS
3. Startet die Anwendung automatisch

**Vorteil:** Kein SD-Karten-Wechsel nötig.  
**Nachteil:** Spiel läuft nur im RAM, keine dauerhafte Installation.

---

## Methode 3: Installation via FTP (ftpd)

Für komfortable Dateiübertragung ohne SD-Kartenentnahme.

### Voraussetzungen
- `ftpd` Homebrew-App auf dem 3DS installiert
- FTP-Client auf dem PC (z. B. FileZilla, WinSCP, oder Windows Explorer)

### Deployment
1. **ftpd** auf dem 3DS starten (über Homebrew Launcher)
2. IP-Adresse notieren (wird auf dem 3DS-Screen angezeigt, z. B. `192.168.1.42:5000`)
3. **FTP-Verbindung vom PC aufbauen:**
   - Host: `ftp://192.168.1.42`
   - Port: `5000`
   - Anonym (kein Passwort)
4. Zu `/3ds/metroid/` navigieren
5. `3ds-cpp.3dsx` und `3ds-cpp.smdh` hochladen
6. FTP-Verbindung trennen, ftpd auf dem 3DS beenden
7. Homebrew Launcher neustarten → Spiel erscheint in der Liste

---

## Fehlerbehebung

### "An exception occurred" beim Start
- **Ursache:** Inkompatible devkitARM-Version oder fehlende Bibliotheken
- **Lösung:** Build mit aktuellem devkitARM neu erstellen

### Spiel erscheint nicht im Homebrew Launcher
- **Ursache:** Falscher Ordner auf der SD-Karte
- **Lösung:** `.3dsx` muss in `/3ds/` oder Unterordner davon liegen

### Schwarzer Bildschirm nach Start
- **Ursache:** Fehlende RomFS-Daten (Maps, Texturen)
- **Lösung:** Sicherstellen, dass `romfs/`-Ordner korrekt gebuildet wurde

### "Failed to load map" Fehler
- **Ursache:** Level-Export nicht ausgeführt
- **Lösung:**
  ```batch
  cd LevelEditor
  python export_maps.py
  cd ..\3ds-cpp
  make
  ```

---

## Verwandte Dokus (Obsidian)
- [[docs/GESAMTDOKU|Gesamtdokumentation (Hub)]]
- [[docs/game/SETUP|Game Setup (Build-Anleitung)]]
- [[docs/tooling/README|Tooling, Build und Skripte]]
- [[docs/quickstart/developer|Quick Start – Entwickler]]
- [[docs/PROGRAMS|Programme und Startskripte]]

---

## Checkliste vor Deployment
- [ ] Level-Export ausgeführt (`LevelEditor/export_maps.py`)
- [ ] 3DS-Build erfolgreich (`make` in `3ds-cpp/`)
- [ ] `.3dsx` und `.smdh` vorhanden
- [ ] SD-Karte hat genug Speicherplatz (mindestens 10 MB frei)
- [ ] Homebrew Launcher auf dem 3DS funktionsfähig
