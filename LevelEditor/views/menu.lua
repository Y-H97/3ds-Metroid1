local State = require "state"
local Constants = require "constants"
local UI = require "ui_components"
local Actions = require "editor_actions"

local M = {}

-- Zeichnet das Hauptmenü
function M.drawMenu()
    love.graphics.print("Hauptmenü", 500, 200)

    -- Buttons wechseln den State direkt
    if UI.drawButton("Editor Starten", 500, 300, 200, 50, nil) then
         State.currentState = Constants.STATE.ROOM_EDIT
    end

    if UI.drawButton("Neuer Raum...", 500, 370, 200, 50, nil) then
         State.currentState = Constants.STATE.NEW_MAP
    end

    if UI.drawButton("Welt Editor", 500, 440, 200, 50, nil) then
         State.currentState = Constants.STATE.WORLD_EDIT
         Actions.refreshWorldFileList() -- Liste laden
         Actions.loadWorld() -- Welt laden
    end
end

-- Zeichnet das Menü zum Erstellen einer neuen Map
function M.drawNewMapMenu()
    love.graphics.print("Wähle Raum Größe:", 50, 50)

    local startX = 50
    local startY = 100
    local lastY = 0

    -- Templates Zeichnen
    for i, tpl in ipairs(Constants.MAP_TEMPLATES) do
        local col = (i-1) % 3
        local row = math.floor((i-1) / 3)

        local x = startX + col * 220
        local y = startY + row * 60
        lastY = y

        -- Da drawButton dauerfeuert wenn gedrückt, müssen wir in main.lua mousepressed verwenden
        -- ODER wir akzeptieren hier, dass createMap mehrmals aufgerufen wird (nicht ideal).
        -- Die originale Logik nutzte mousepressed für die Action, drawButton nur für Darstellung.
        -- Wir nutzen hier drawButton als reine Visualisierung.
        UI.drawButton(tpl.name, x, y, 200, 50, nil)
    end

    -- Custom Size UI
    local customY = lastY + 80
    love.graphics.print("Benutzerdefiniert: " .. State.customMapW .. " x " .. State.customMapH, 50, customY)

    -- Plus/Minus Buttons (Hier können wir IMGUI nutzen, da State-Change billig ist)
    -- Verwenden Timer um zu schnelles toggeln zu verhindern? Nein, original war mousepressed.
    -- Wir zeichnen sie nur, Logik bleibt in main.lua mousepressed für Präzision.
    UI.drawButton("-", 50, customY + 30, 30, 30)
    UI.drawButton("+", 90, customY + 30, 30, 30)

    UI.drawButton("-", 150, customY + 30, 30, 30)
    UI.drawButton("+", 190, customY + 30, 30, 30)

    UI.drawButton("Erstellen ("..State.customMapW.."x"..State.customMapH..")", 250, customY + 25, 200, 40)
    UI.drawButton("Zurück", 50, customY + 100, 200, 50, nil)
end

return M
