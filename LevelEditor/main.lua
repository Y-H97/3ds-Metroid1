-- Metroidvania Level Editor (Refactored)
-- Haupt-Einstiegspunkt
-- Koordiniert die verschiedenen Module (State, Views, Actions).

local State = require "state"
local Constants = require "constants"
local Views = require "views"
local Actions = require "editor_actions"
local IOUtils = require "io_utils"
local UI = require "ui_components"
local MapExporter = require "map_exporter"

-- Layout Konstanten (müssen mit Views übereinstimmen)
local UI_WIDTH = 200
local PANEL_X = 220 

-- Sucht in der Raumliste nach einem Eintrag über den Dateinamen.
local function findRoomFileByName(name)
    for _, fileData in ipairs(State.worldFiles) do
        if fileData.name == name then return fileData end
    end
    return nil
end

local function worldCellFromMouse(mx, my)
    -- Wandelt Mauskoordinaten in Welt-Grid-Koordinaten um.
    local gridOffsetX = 220
    local gridOffsetY = 50
    local cellSize = 100

    if mx < gridOffsetX then return nil, nil end

    local worldMX = ((mx - gridOffsetX) / State.zoom) + State.camX
    local worldMY = ((my - gridOffsetY) / State.zoom) + State.camY

    local gx = math.floor(worldMX / cellSize)
    local gy = math.floor(worldMY / cellSize)
    return gx, gy
end

local function findPlacedRoomAt(gx, gy)
    -- Prüft, ob auf der Weltzelle bereits ein (ggf. mehrzelliger) Raum liegt.
    for key, roomName in pairs(State.worldGrid) do
        local ox, oy = key:match("(%-?%d+),(%-?%d+)")
        if ox and oy then
            ox, oy = tonumber(ox), tonumber(oy)
            local meta = State.roomCache[roomName] or {w = 1, h = 1}
            if gx >= ox and gx < (ox + meta.w) and gy >= oy and gy < (oy + meta.h) then
                return key, roomName, ox, oy, meta.w, meta.h
            end
        end
    end
    return nil
end

local function canPlaceRoom(roomName, gx, gy)
    -- Validiert Platzierung: innerhalb der Grenzen und ohne Überlappung.
    local meta = State.roomCache[roomName] or {w = 1, h = 1}
    local w, h = meta.w, meta.h

    if gx < 0 or gy < 0 or gx + w > State.worldGridSize or gy + h > State.worldGridSize then
        return false
    end

    for key, otherName in pairs(State.worldGrid) do
        local ox, oy = key:match("(%-?%d+),(%-?%d+)")
        if ox and oy then
            ox, oy = tonumber(ox), tonumber(oy)
            local other = State.roomCache[otherName] or {w = 1, h = 1}
            local noOverlap = (gx + w <= ox) or (gx >= ox + other.w) or (gy + h <= oy) or (gy >= oy + other.h)
            if not noOverlap then
                return false
            end
        end
    end

    return true
end

function love.load(args)
    -- Startpunkt der App; optionaler Headless-Export mit --export-json.
    if args and args[1] == "--export-json" then
        local ok, message = MapExporter.exportAll()
        if ok then
            print(message)
            love.event.quit(0)
        else
            print("Export Fehler: " .. tostring(message))
            love.event.quit(1)
        end
        return
    end

    love.window.setTitle("Metroidvania Level Editor (Refactored)")
    love.window.setMode(1280, 720, {resizable=true})
    
    -- Initialisiere eine Standard-Map
    Actions.createMap(1, 1)
end

function love.update(dt)
    -- Zyklisches Update für Timer, Kamera und Editor-Eingaben.
    -- 1. Fullscreen Restore Hack
    if State.pendingFullscreen then
        love.window.setFullscreen(true)
        State.pendingFullscreen = false
    end
    
    -- 2. Nachricht Timer
    if State.messageTimer > 0 then
        State.messageTimer = State.messageTimer - dt
        if State.messageTimer < 0 then State.message = "" end
    end
    
    -- 3. Kamera Bewegung
    if State.currentState == Constants.STATE.ROOM_EDIT or State.currentState == Constants.STATE.WORLD_EDIT then
        if love.keyboard.isDown("up") then State.camY = State.camY - State.camSpeed * dt end
        if love.keyboard.isDown("down") then State.camY = State.camY + State.camSpeed * dt end
        if love.keyboard.isDown("left") then State.camX = State.camX - State.camSpeed * dt end
        if love.keyboard.isDown("right") then State.camX = State.camX + State.camSpeed * dt end
    end
    
    -- 4. Raum Editor Interaktion (Klicken & Malen)
    if State.currentState == Constants.STATE.ROOM_EDIT then
        handleRoomEditorInput()
    end
end

function love.draw()
    -- Zeichnet je nach App-Modus die passende Ansicht.
    love.graphics.push()
    
    if State.currentState == Constants.STATE.MENU then
        Views.drawMenu()
    elseif State.currentState == Constants.STATE.NEW_MAP then
        Views.drawNewMapMenu()
    elseif State.currentState == Constants.STATE.ROOM_EDIT then
        Views.drawRoomEditor()
    elseif State.currentState == Constants.STATE.WORLD_EDIT then
        Views.drawWorldEditor()
    end
    
    love.graphics.pop()
end

-- Lagert die Maus-Logik für den Raum-Editor aus update/draw aus
function handleRoomEditorInput()
    local mx, my = love.mouse.getPosition()
    
    -- A: Interaktion mit Sidebar (Links)
    -- UI Elemente in Views werden via drawButton handled, aber die Tile-Liste ist speziell
    if mx < UI_WIDTH and my > 65 then
        if love.mouse.isDown(1) then
            local relativeY = my - 70 + State.listScroll
            local idx = math.floor(relativeY / 40) + 1
            if Constants.BLOCK_TYPES[idx] then
                State.currentTileType = Constants.BLOCK_TYPES[idx].id
            end
        end
        
    -- B: Interaktion mit Map (Rechts)
    elseif mx >= PANEL_X then
        -- Welt-Koordinaten berechnen
        -- Formel: World = ((Screen - UIOffset) / Zoom) + Cam
        -- Offset in View ist (PANEL_X, 80) dann scale dann -cam
        -- ABER: in Views.drawRoomEditor ist translations-Reihenfolge:
        -- translate(PANEL_X, 80) -> scale(zoom) -> translate(-camX, -camY)
        -- Inverse:
        -- 1. Screen - Offset
        local localX = mx - PANEL_X
        local localY = my - 80
        -- 2. / Zoom
        local unscaledX = localX / State.zoom
        local unscaledY = localY / State.zoom
        -- 3. + Cam
        local worldMX = unscaledX + State.camX - 50 -- minus mapOffsetX (50) aus View
        local worldMY = unscaledY + State.camY - 50 -- minus mapOffsetY (50) aus View
        
        -- Mapping/Painting (Tile-only)
        if love.mouse.isDown(1) or love.mouse.isDown(2) then
            local tx = math.floor(worldMX / Constants.TILE_SIZE) + 1
            local ty = math.floor(worldMY / Constants.TILE_SIZE) + 1

            if State.currentRoom[ty] and State.currentRoom[ty][tx] then
                if love.mouse.isDown(1) then State.currentRoom[ty][tx] = State.currentTileType end
                if love.mouse.isDown(2) then State.currentRoom[ty][tx] = 0 end -- Löschen
            end
        end
    end
end

function love.mousepressed(x, y, button)
    -- Behandelt Einmalklicks (Buttons, Auswahl, Weltplatzierung).
    -- Menü-Buttons und "Einmalklicks"
    if State.currentState == Constants.STATE.MENU then
        -- Logik ist jetzt in Views.drawMenu via drawButton (state change sofort)
        -- Daher hier nichts zu tun, außer wir wollen Klick-Sound
    
    elseif State.currentState == Constants.STATE.NEW_MAP then
        -- Custom Size Controls (Plus/Minus)
        -- Da drawButton in main loop feuert, machen wir präzise Klicks hier
        -- Layout Annahme: Wie in Views.drawNewMapMenu
        -- Wir müssen das Layout "wissen" oder drawButton besser nutzen.
        -- Einfacher: Wir prüfen Regionen grob, da es nur ein Editor ist.
        
        local customY = 100 + (math.ceil(#Constants.MAP_TEMPLATES/3) * 60) + 80
        -- CustomY berechnung ist complex, wir holen es uns statisch oder vereinfachen
        -- Da drawNewMapMenu dynamisch ist, ist das fehleranfällig. 
        -- Lösung: Wir machen das in update() via drawButton returns true (schon implemented in Views)
        -- ABER: drawButton feuert jeden Frame. Ein kurzer Klick ist mehrere Frames.
        -- Besser: mousepressed nutzen.
        
        -- Da die Logik komplex ist, lassen wir NewMap Action logik hier minimal oder ignorieren sie,
        -- WENN wir drawButton in Update/Draw nutzen.
        -- Im View feuert drawButton kontinuierlich. Das ist OK für Tiles, schlecht für Buttons.
        -- UI.drawButton prüft "love.mouse.isDown(1)".
        -- Das refactoring zu "OnRelease" oder "OnPress" wäre besser.
        
        -- Template Selection Logik von früher übernehmen:
        local startX, startY = 50, 100
        for i, tpl in ipairs(Constants.MAP_TEMPLATES) do
            local col = (i-1) % 3
            local row = math.floor((i-1) / 3)
            local bx = startX + col * 220
            local by = startY + row * 60
            
            if x >= bx and x <= bx + 200 and y >= by and y <= by + 50 then
                Actions.createMap(tpl.w, tpl.h)
                State.currentState = Constants.STATE.ROOM_EDIT
                Actions.setMessage("Neuer Raum erstellt: " .. tpl.name, 3)
                return
            end
        end
        
        -- Custom + / - und Erstellen
        if y >= customY + 30 and y <= customY + 60 then
            if x >= 50 and x <= 80 then State.customMapW = math.max(1, State.customMapW - 1); return end
            if x >= 90 and x <= 120 then State.customMapW = math.min(20, State.customMapW + 1); return end
            if x >= 150 and x <= 180 then State.customMapH = math.max(1, State.customMapH - 1); return end
            if x >= 190 and x <= 220 then State.customMapH = math.min(20, State.customMapH + 1); return end
        end

        if x >= 250 and x <= 450 and y >= customY + 25 and y <= customY + 65 then
            Actions.createMap(State.customMapW, State.customMapH)
            State.currentState = Constants.STATE.ROOM_EDIT
            Actions.setMessage("Neuer Raum erstellt: " .. State.customMapW .. "x" .. State.customMapH, 3)
            return
        end

        if x >= 50 and x <= 250 and y >= customY + 100 and y <= customY + 150 then
            State.currentState = Constants.STATE.MENU
            return
        end

    elseif State.currentState == Constants.STATE.ROOM_EDIT then
        if button == 1 and x < UI_WIDTH and y > 65 then
            local relativeY = y - 70 + State.listScroll
            local idx = math.floor(relativeY / 40) + 1
            if Constants.BLOCK_TYPES[idx] then
                State.currentTileType = Constants.BLOCK_TYPES[idx].id
            end
        end

    elseif State.currentState == Constants.STATE.WORLD_EDIT then
        local listX, listY, listW = 10, 50, 200

        if button == 1 and x >= listX and x <= listX + listW and y >= listY then
            local idx = math.floor((y - 80) / 20) + 1
            if idx >= 1 and idx <= #State.worldFiles then
                State.selectedWorldFile = State.worldFiles[idx]
                Actions.setMessage("Ausgewaehlt: " .. State.selectedWorldFile.name, 1.5)
            end
            return
        end

        local gx, gy = worldCellFromMouse(x, y)
        if not gx or not gy then return end
        if gx < 0 or gx >= State.worldGridSize or gy < 0 or gy >= State.worldGridSize then return end

        if button == 1 then
            if State.selectedWorldFile then
                if canPlaceRoom(State.selectedWorldFile.name, gx, gy) then
                    local keyCoord = gx .. "," .. gy
                    State.worldGrid[keyCoord] = State.selectedWorldFile.name
                    Actions.setMessage("Raum platziert: " .. State.selectedWorldFile.name, 1.5)
                else
                    Actions.setMessage("Kann dort nicht platziert werden", 1.5)
                end
            else
                local hitKey, hitName = findPlacedRoomAt(gx, gy)
                if hitKey and hitName then
                    State.worldGrid[hitKey] = nil
                    State.worldCheckpoints[hitKey] = nil
                    State.selectedWorldFile = findRoomFileByName(hitName) or {name = hitName, w = 1, h = 1}
                    Actions.setMessage("Raum aufgenommen: " .. hitName .. " (neu platzieren)", 2)
                end
            end
        elseif button == 2 then
            local hitKey, hitName = findPlacedRoomAt(gx, gy)
            if hitKey then
                State.worldGrid[hitKey] = nil
                State.worldCheckpoints[hitKey] = nil
                Actions.setMessage("Raum entfernt: " .. hitName, 1.5)
            end
        end
    end
end

function love.keypressed(key)
    -- Tastaturkürzel für Moduswechsel, Save/Load und Editor-Aktionen.
    if key == "f11" then
        love.window.setFullscreen(not love.window.getFullscreen())
    end

    if key == "escape" then
        if State.currentState == Constants.STATE.MENU then
            love.event.quit()
        else
            State.currentState = Constants.STATE.MENU
        end
    end
    
    if State.currentState == Constants.STATE.ROOM_EDIT then
        if key == "1" then State.currentTileType = 1 end
        if key == "2" then State.currentTileType = 2 end
        if key == "3" then State.currentTileType = 3 end
        
        if key == "s" then
            local path = IOUtils.showSaveDialog()
            if path then
                Actions.saveRoom(path)
                State.currentFilename = path:match("([^\\]+)$") or path
                Actions.setMessage("Gespeichert!", 3)
            end
        elseif key == "l" then
            local path = IOUtils.showOpenDialog()
            if path then
                local content = IOUtils.readFilePS(path)
                if content then
                    local chunk, err = load(content) 
                    if chunk then
                        local result = chunk()
                        if result then
                            if result.grid then
                                State.currentRoom = result.grid
                            else
                                State.currentRoom = result
                            end
                            State.currentFilename = path:match("([^\\]+)$") or path
                            Actions.setMessage("Geladen!", 3)
                        end
                    end
                end
            end
        end
    end
    
    if State.currentState == Constants.STATE.WORLD_EDIT then
        if key == "s" then
            Actions.saveWorld()
        elseif key == "l" then
            Actions.loadWorld()
        elseif key == "r" then
            Actions.refreshWorldFileList()
            Actions.setMessage("Liste aktualisiert!", 3)
        elseif key == "c" then
            -- Toggle Checkpoint
            local mx, my = love.mouse.getPosition()
            local gridOffsetX = 220
            local gridOffsetY = 50
            local cellSize = 100
            
            if mx >= gridOffsetX then
                local worldMX = ((mx - gridOffsetX) / State.zoom) + State.camX
                local worldMY = ((my - gridOffsetY) / State.zoom) + State.camY
                
                local gx = math.floor( worldMX / cellSize )
                local gy = math.floor( worldMY / cellSize )
                
                if gx >= 0 and gx < State.worldGridSize and gy >= 0 and gy < State.worldGridSize then
                    local keyCoord = gx .. "," .. gy
                    if State.worldGrid[keyCoord] then
                        State.worldCheckpoints[keyCoord] = not State.worldCheckpoints[keyCoord]
                        Actions.setMessage(State.worldCheckpoints[keyCoord] and "Checkpoint gesetzt!" or "Checkpoint entfernt!", 1)
                    end
                end
            end
        end
    end
end

function love.wheelmoved(x, y)
    -- Mausrad für Listen-Scroll und Zoomsteuerung.
    local mx, my = love.mouse.getPosition()
    
    if State.currentState == Constants.STATE.ROOM_EDIT then
        if mx < PANEL_X then
            State.listScroll = State.listScroll - y * 30
            if State.listScroll < 0 then State.listScroll = 0 end
        else
            if y > 0 then State.zoom = State.zoom + 0.1
            elseif y < 0 then State.zoom = State.zoom - 0.1 end
        end
    elseif State.currentState == Constants.STATE.WORLD_EDIT then
        if y > 0 then State.zoom = State.zoom + 0.1
        elseif y < 0 then State.zoom = State.zoom - 0.1 end
    end
    
    -- Zoom Limits
    if State.zoom < 0.2 then State.zoom = 0.2 end
    if State.zoom > 3.0 then State.zoom = 3.0 end
end
