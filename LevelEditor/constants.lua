-- Level Editor Konstatne & Definitionen
-- Enthält feste Werte wie Farben, Block-Typen und UI-Einstellungen.

local Constants = {}

-- 3DS Auflösung (Top Screen Basis, für die Simulation der Map-Screens)
Constants.SCREEN_W_PX = 400
Constants.SCREEN_H_PX = 240
Constants.TILE_SIZE = 16

-- Berechnete Grid-Größen
Constants.TILES_X = Constants.SCREEN_W_PX / Constants.TILE_SIZE -- 25
Constants.TILES_Y = Constants.SCREEN_H_PX / Constants.TILE_SIZE -- 15

-- UI-Farben und Block-Definitionen
-- Diese Liste definiert, welche Tiles im Editor platziert werden können.
Constants.BLOCK_TYPES = {
    {id=1, name="Wand (Solid)", color={0.5, 0.5, 0.8}},
    {id=2, name="Gefahr (Rot)", color={0.9, 0.2, 0.2}},
    {id=3, name="Trans. (Grün)", color={0.2, 0.9, 0.2}},
    
    -- Slopes (Schrägen / Dreiecke)
    -- shape gibt den Typ der Schräge an, wichtig für die Kollisionsphysik
    {id=30, name="Rampe /",      color={0.6, 0.6, 0.9}, shape="slope_ur"}, -- Up-Right: Steigt nach rechts an
    {id=31, name="Rampe \\",     color={0.6, 0.6, 0.9}, shape="slope_ul"}, -- Up-Left: Steigt nach links an
    {id=32, name="Decke /",      color={0.4, 0.4, 0.7}, shape="slope_dl"}, -- Down-Left: Decke sinkt nach links
    {id=33, name="Decke \\",     color={0.4, 0.4, 0.7}, shape="slope_dr"}, -- Down-Right: Decke sinkt nach rechts
    
    -- Dekorative Hintergrund-Farben (Graustufen)
    {id=4, name="HG Grau",       color={0.2, 0.2, 0.2}},
    {id=8, name="HG Dunkelgrau", color={0.15, 0.15, 0.15}},
    {id=9, name="HG Tiefschwarz",color={0.08, 0.08, 0.08}},
    {id=10, name="HG Schiefer",  color={0.18, 0.22, 0.25}},
    
    -- Blautöne
    {id=5, name="HG Navy",       color={0.1, 0.1, 0.3}},
    {id=11, name="HG Nachtblau", color={0.05, 0.05, 0.2}},
    {id=12, name="HG Petrol",    color={0.0, 0.2, 0.25}},
    {id=19, name="HG Türkis D.", color={0.0, 0.25, 0.25}},
    
    -- Grüntöne
    {id=6, name="HG Dunkelgrün", color={0.1, 0.2, 0.1}},
    {id=13, name="HG Wald",      color={0.05, 0.15, 0.05}},
    {id=14, name="HG Oliv",      color={0.2, 0.2, 0.1}},
    {id=20, name="HG Sumpf",     color={0.15, 0.18, 0.12}},
    
    -- Warme Töne (Rot/Braun/Lila)
    {id=7, name="HG Lila",       color={0.2, 0.1, 0.2}},
    {id=15, name="HG Weinrot",   color={0.3, 0.1, 0.1}},
    {id=16, name="HG Braun",     color={0.3, 0.2, 0.1}},
    {id=17, name="HG Moor",      color={0.2, 0.15, 0.08}},
    {id=18, name="HG Mahagoni",  color={0.25, 0.1, 0.05}},
    {id=21, name="HG Ziegel D.", color={0.35, 0.15, 0.1}},
}

-- Vorlagen für Kartengrößen (in Bildschirmen/Screens)
Constants.MAP_TEMPLATES = {
    {name = "1x1 (Standard)", w=1, h=1},
    {name = "2x2 (Quadrat)", w=2, h=2},
    {name = "3x3 (Mittel)", w=3, h=3},
    {name = "4x4 (Groß)", w=4, h=4},
    {name = "5x5 (Riesig)", w=5, h=5},
    {name = "6x6 (Max)", w=6, h=6},
    
    {name = "2x1 (Breit)", w=2, h=1},
    {name = "3x1 (Breit)", w=3, h=1},
    {name = "4x1 (Breit)", w=4, h=1},
    {name = "6x1 (Breit)", w=6, h=1},

    {name = "1x2 (Hoch)", w=1, h=2},
    {name = "1x3 (Hoch)", w=1, h=3},
    {name = "1x4 (Hoch)", w=1, h=4},
    {name = "1x6 (Hoch)", w=1, h=6},
    
    {name = "3x2 (Quer)", w=3, h=2},
    {name = "2x3 (Hoch)", w=2, h=3},
}

-- Editor-Zustände (Enum)
Constants.STATE = {
    MENU = "menu",
    NEW_MAP = "new_map",
    ROOM_EDIT = "room_edit",
    WORLD_EDIT = "world_edit"
}

return Constants
