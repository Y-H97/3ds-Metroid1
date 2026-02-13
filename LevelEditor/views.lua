local State = require "state"
local Constants = require "constants"
local UI = require "ui_components"
local Actions = require "editor_actions" -- Wird benötigt für Action-Aufrufe (z.B. createMap aus Button)

local Views = {}

-- Zeichnet das Hauptmenü
function Views.drawMenu()
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
function Views.drawNewMapMenu()
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

-- Hilfsfunktion für RoomEditor: Zeichnet die Shapes/Polygone
local function drawShape(x, y, w, h, shape, mode)
    local x1, y1, x2, y2, x3, y3
    
    if shape == "slope_ur" then -- Up-Right /|
         x1, y1 = x, y + h
         x2, y2 = x + w, y + h
         x3, y3 = x + w, y
    elseif shape == "slope_ul" then -- Up-Left |\
         x1, y1 = x, y
         x2, y2 = x, y + h
         x3, y3 = x + w, y + h
    elseif shape == "slope_dl" then -- Down-Left |/
         x1, y1 = x, y
         x2, y2 = x, y + h
         x3, y3 = x + w, y
    elseif shape == "slope_dr" then -- Down-Right \|
          x1, y1 = x, y
          x2, y2 = x + w, y + h
          x3, y3 = x + w, y
    end
    
    if x1 then
        love.graphics.polygon(mode, x1, y1, x2, y2, x3, y3)
    end
end

-- === HAUPT EDITOR VIEW ===
function Views.drawRoomEditor()
    local rightPanelX = 220 
    local uiW = 200
    local sw, sh = love.graphics.getDimensions()

    -- Header Infos
    love.graphics.setColor(1, 1, 1)
    love.graphics.print("[S] Speichern | [L] Laden | [ESC] Menü | [F11] Vollbild", rightPanelX + 10, 10)
    love.graphics.print("Raum: " .. UI.sanitizeForDisplay(State.currentFilename) .. " (" .. State.mapScreensW .. "x" .. State.mapScreensH .. ")", rightPanelX + 10, 30)
    
    if State.message ~= "" then
        love.graphics.setColor(0, 1, 0)
        love.graphics.print(UI.sanitizeForDisplay(State.message), rightPanelX + 10, 50)
        love.graphics.setColor(1, 1, 1)
    end
    
    -- Sidebar Hintergrund
    love.graphics.setColor(0.15, 0.15, 0.15)
    love.graphics.rectangle("fill", 0, 0, uiW, sh)
    love.graphics.setColor(1, 1, 1)
    love.graphics.rectangle("line", 0, 0, uiW, sh)
    
     love.graphics.print("Tiles:", 10, 45)
    
    -- Liste der Blöcke
    love.graphics.setScissor(0, 65, uiW, sh - 150)
        local startY = 70 - State.listScroll
        
        for i, block in ipairs(Constants.BLOCK_TYPES) do
            local y = startY + (i-1) * 40
            
            -- Ausgewählt Highlight
            if State.currentTileType == block.id then
                love.graphics.setColor(0.4, 0.4, 0.4)
                love.graphics.rectangle("fill", 5, y-5, uiW-10, 35)
                love.graphics.setColor(1, 1, 0)
                love.graphics.rectangle("line", 5, y-5, uiW-10, 35)
            end
            
            -- Farbe Vorschau
            if block.color then
                love.graphics.setColor(block.color)
            else
                love.graphics.setColor(1, 1, 1)
            end
            love.graphics.rectangle("fill", 10, y, 20, 20)
            
            -- Name
            love.graphics.setColor(1, 1, 1)
            love.graphics.print(block.name, 40, y + 2)
        end
    love.graphics.setScissor()
    
    -- Map Renderer
    local mapW = sw - rightPanelX
    local mapH = sh
    
    love.graphics.setScissor(rightPanelX, 0, mapW, mapH)
    love.graphics.push()
    love.graphics.translate(rightPanelX, 80)
    love.graphics.scale(State.zoom)
    love.graphics.translate(-State.camX, -State.camY)
    
    local mapOffsetX, mapOffsetY = 50, 50 
    
    -- Hilfsgitter Screens
    love.graphics.setColor(0.3, 0.3, 0.3)
    for sy=0, State.mapScreensH-1 do
        for sx=0, State.mapScreensW-1 do
            local rx = mapOffsetX + sx * Constants.SCREEN_W_PX
            local ry = mapOffsetY + sy * Constants.SCREEN_H_PX
            love.graphics.rectangle("line", rx, ry, Constants.SCREEN_W_PX, Constants.SCREEN_H_PX)
            love.graphics.print("Screen " .. sx .. "," .. sy, rx + 10, ry + 10)
        end
    end
    
    -- Map Rahmen
    love.graphics.setColor(1, 1, 1)
    love.graphics.rectangle("line", mapOffsetX, mapOffsetY, State.mapScreensW * Constants.SCREEN_W_PX, State.mapScreensH * Constants.SCREEN_H_PX)
    
    -- Tiles Rendern
    for y, row in ipairs(State.currentRoom) do
        for x, tile in ipairs(row) do
            if tile ~= 0 then
                local color = {1, 0, 1}
                local shape = "rect"
                for _, b in ipairs(Constants.BLOCK_TYPES) do
                    if b.id == tile then 
                        color = b.color
                        if b.shape then shape = b.shape end
                        break 
                    end
                end
                
                love.graphics.setColor(color)
                local drawX = mapOffsetX + (x-1)*Constants.TILE_SIZE
                local drawY = mapOffsetY + (y-1)*Constants.TILE_SIZE
                
                if shape == "rect" then
                    love.graphics.rectangle("fill", drawX, drawY, Constants.TILE_SIZE, Constants.TILE_SIZE)
                else
                    drawShape(drawX, drawY, Constants.TILE_SIZE, Constants.TILE_SIZE, shape, "fill")
                end
            end
        end
    end
    
    love.graphics.setColor(1,1,1)
    love.graphics.pop()
    love.graphics.setScissor()
end

-- === WELT EDITOR VIEW ===
function Views.drawWorldEditor()
    local sw, sh = love.graphics.getDimensions()
    love.graphics.print("Welt Editor - [S] Speichern | [L] Welt Laden | [R] Refresh Liste | [C] Checkpoint (Maus über Raum) | [ESC] Menü | [F11] Vollbild", 10, 10)
    
    if State.message ~= "" then
        love.graphics.setColor(0, 1, 0)
        love.graphics.print(UI.sanitizeForDisplay(State.message), 10, 30)
        love.graphics.setColor(1, 1, 1)
    end
    
    local listW = 200
    local gridOffsetX = 220
    local gridOffsetY = 50
    local listH = sh - 60
    
    -- Dateiliste
    love.graphics.setColor(0.2, 0.2, 0.2)
    love.graphics.rectangle("fill", 10, 50, listW, listH)
    love.graphics.setColor(1, 1, 1)
    love.graphics.rectangle("line", 10, 50, listW, listH)
    
    love.graphics.setScissor(10, 50, listW, listH)
        love.graphics.print("Verfügbare Level:", 15, 55)
        
        for i, fileData in ipairs(State.worldFiles) do
            local y = 80 + (i-1) * 20
            local displayText = UI.sanitizeForDisplay(fileData.name) .. " (" .. fileData.w .. "x" .. fileData.h .. ")"
            
            if fileData == State.selectedWorldFile then
                love.graphics.setColor(0, 1, 0)
                love.graphics.print("> " .. displayText, 15, y)
            else
                love.graphics.setColor(0.8, 0.8, 0.8)
                love.graphics.print(displayText, 15, y)
            end
        end
    love.graphics.setScissor()
    
    love.graphics.setColor(1, 1, 1)
    
    -- Grid Darstellung
    local gridW = sw - gridOffsetX
    local gridH = sh - gridOffsetY
    
    love.graphics.setScissor(gridOffsetX, gridOffsetY, gridW, gridH)
    love.graphics.push()
    love.graphics.translate(gridOffsetX, gridOffsetY)
    love.graphics.scale(State.zoom)
    love.graphics.translate(-State.camX, -State.camY)
    
    local cellSize = 100
    
    -- Gitter-Linien
    love.graphics.setColor(0.3, 0.3, 0.3)
    for y=0, State.worldGridSize-1 do
        for x=0, State.worldGridSize-1 do
            local rx = x * cellSize
            local ry = y * cellSize
            love.graphics.rectangle("line", rx, ry, cellSize, cellSize)
        end
    end
    
    -- Räume im Grid
    for key, roomName in pairs(State.worldGrid) do
        local x, y = key:match("(%-?%d+),(%-?%d+)")
        if x and y then
            x, y = tonumber(x), tonumber(y)
            local rx = x * cellSize
            local ry = y * cellSize
            
            -- Größe aus Cache abrufen
            local w, h = 1, 1
            if State.roomCache[roomName] then
                w = State.roomCache[roomName].w
                h = State.roomCache[roomName].h
            end
            
            local isCheckpoint = State.worldCheckpoints[key]
            
            if isCheckpoint then
                 love.graphics.setColor(0.8, 0.6, 0.2)
            else
                 love.graphics.setColor(0.2, 0.4, 0.8)
            end
            
            love.graphics.rectangle("fill", rx + 5, ry + 5, (w * cellSize) - 10, (h * cellSize) - 10)
            
            love.graphics.setColor(1, 1, 1)
            -- Einfache Textanzeige
            if w >= 1 then
                love.graphics.print(UI.sanitizeForDisplay(roomName), rx + 10, ry + 20)
                love.graphics.print("("..w.."x"..h..")", rx + 10, ry + 40)
                if isCheckpoint then
                    love.graphics.print("[CP]", rx + 10, ry + 60)
                end
            end
        end
    end

    love.graphics.setColor(1, 1, 1)
    love.graphics.pop()
    love.graphics.setScissor()
end

return Views
