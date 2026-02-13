local Constants = require "constants"

-- Zentraler Speicher für den Editor-Zustand (Mutable State)
-- Hier liegen alle Variablen, die sich während der Laufzeit ändern.
local State = {
    -- Aktueller Modus (Startet im Menü)
    currentState = Constants.STATE.MENU,
    
    -- UI Flags
    pendingFullscreen = false, -- Flag zum Wiederherstellen des Vollbilds nach Dialogen
    listScroll = 0, -- Scroll-Offset für die Tile-Liste
    
    -- Benachrichtigungssystem (Grüner Text oben)
    message = "",
    messageTimer = 0,
    
    -- Kamera-Einstellungen (für Pan & Zoom im Editor)
    camX = 0,
    camY = 0,
    zoom = 1.0,
    camSpeed = 300, -- Pixel pro Sekunde bei Tastensteuerung

    -- === Raum Editor Daten ===
    currentRoom = {}, -- 2D Array [y][x] für Tiles
    currentFilename = "level1",
    
    mapScreensW = 1, -- Breite in Screens
    mapScreensH = 1, -- Höhe in Screens

    -- Werkzeug-Einstellungen
    currentTool = "tile",
    currentTileType = 1, -- ID des aktuell gewählten Tiles

    -- Custom Map Größe (temporär für UI)
    customMapW = 2,
    customMapH = 2,

    -- === Welt Editor Daten ===
    worldGrid = {}, -- [ "x,y" ] = filename
    worldFiles = {}, -- Liste aller verfügbaren .lua Level-Dateien im Ordner
    selectedWorldFile = nil, -- Aktuell in der Liste ausgewählte Datei
    worldCheckpoints = {}, -- [ "x,y" ] = boolean (Ist dieser Raum ein Checkpoint?)
    roomCache = {}, -- Cache für Raum-Größen (um im Grid richtige Boxen zu zeichnen)
    worldGridSize = 20 -- Größe des Welt-Rasters (20x20 Zellen)
}

return State
