# Item-System

In dieser Version wurde ein rudimentäres Item-/Inventarsystem ergänzt. Schritte:

1. **Editor**
   * Neuer Werkzeugmodus `Item` (Taste `i` umschalten).
   * Items werden als blaues Quadrat auf dem Raum platziert.
   * Zur Zeit gibt es nur einen Typ (`Doppelsprung`) – weitere Typen können
     in `LevelEditor/constants.lua` ergänzt werden.
   * Beim Speichern des Raumes werden sämtliche Items in `items` exportiert.
  * Der Map-Exporter (`LevelEditor/map_exporter.lua`, `LevelEditor/export_maps.py`
    und das PowerShell-Skript `LevelEditor/export_maps.ps1`) schreibt die Items
    automatisch in das JSON-Format – der Batch‑Export (`run_export_maps.bat`)
    ist somit ebenfalls kompatibel.

2. **Engine (3DS + Simulator)**
   * Neue Daten in `gameplay_scene` und Simulator: `collectedItems`, `activeItems`.
   * Karten (`TileMap`) enthalten nun ein zusätzliches Feld `items`.
   * Beim Laden eines Raumes werden nicht-eingesammelte Items als `MapItem`
     intern angelegt und beim Kontakt mit dem Spieler eingesammelt.
   * Das `double_jump`-Item ist in `source/gameplay/items/double_jump` gekapselt.
     Weitere Items können analog in eigenen Unterverzeichnissen implementiert
     werden.
* Beim Laden einer Karte konvertiert die Engine inzwischen automatisch
     Editor-Platzhalterkacheln (z.B. Tile-ID 99 für Doppelsprung) in echte
     Items, falls beim Export das `items`-Array fehlt. Dadurch tauchen die
     blauen Quadrate im Simulator nicht mehr als unerreichbare Wände auf.
   * Beim Einsammeln wird der Status persistiert und eine Nachricht angezeigt.
   * Bottom-Screen der Engine zeigt Inventar mit ON/OFF-Schalter für jedes
     gesammelte Item (mit Richtungs-Tasten und `Y` toggeln). In der Simulation
     und auf dem 3DS kann der Schalter jetzt auch per Touch angeklickt werden
     – einfach auf den Eintrag im unteren Bildschirm tippen, um ihn ein-/auszuschalten.
   * Persistente Speicherdateien (`savegame_slotX.dat`) enthalten jetzt eine
     zusätzliche Zeile `items <collected> <active>`.
   * Simulator spiegelt das Verhalten inklusive Inventar-UI und Map-Cache-Ausgabe.

3. **Persistenz & Testing**
   * Neue `ITEM_DOUBLE_JUMP`-Flag in `GameplayScene` (bitweise Masken).
   * Eigene Methoden `grantDoubleJump()` / `playerHasDoubleJump()` vereinfachen
     das Hinzufügen weiterer Effekte.
   * Beim Start eines neuen Spiels werden alle Item-Zustände zurückgesetzt.
   * Beim Laden eines Savegames werden Item-Zustände übernommen und Effekte
     angewendet.

## Entwickeln weiterer Items

* Anlegen eines neuen Ordners unter `source/gameplay/items/`.
* Implementieren einer `id()`-Funktion und `onCollect(GameplayScene&)` in der
  jeweiligen Datei.
* Map-Export/Editor muss ggf. um neue `ITEM_TYPES`-Einträge erweitert werden.
* In `GameplayScene::refreshMapItems` und beim Einsammeln entsprechende
  Branches hinzufügen (oder zentraleren Dispatcher einbauen).

Weitere Details und Erweiterungsmöglichkeiten siehe [[docs/GESAMTDOKU]].
