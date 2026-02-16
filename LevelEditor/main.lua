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
    -- Start im Vollbildmodus (desktop fullscreen), behalte Resizability falls user wechselt
    love.window.setMode(1280, 720, {resizable=true, fullscreen=true, fullscreentype="desktop"})

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

    -- 2b. Hilfe Overlay Timer (zeigt die Erklärung kurz beim Start)
    if State.showHelpTimer and State.showHelpTimer > 0 then
        State.showHelpTimer = State.showHelpTimer - dt
        if State.showHelpTimer <= 0 then State.showHelp = false end
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

    -- Caret blink für das Suchfeld
    if State.tileSearchFocus then
        State.tileSearchCaretTimer = (State.tileSearchCaretTimer or 0.5) - dt
        if State.tileSearchCaretTimer <= 0 then
            State.tileSearchCaretTimer = 0.5
            State.tileSearchCaretVisible = not State.tileSearchCaretVisible
        end
    else
        State.tileSearchCaretVisible = false
        State.tileSearchCaretTimer = 0.5
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
        -- Scroll- / Suche-Bereich berücksichtigen (Search box ist bei y=70)
        if love.mouse.isDown(1) then
            -- Gefilterte Liste berechnen (wie in Views.drawRoomEditor)
            local displayed = {}
            local filter = (State.tileSearch or ""):lower()
            for i, block in ipairs(Constants.BLOCK_TYPES) do
                if filter == "" or (block.name and block.name:lower():find(filter, 1, true)) then
                    table.insert(displayed, block)
                end
            end

            -- relativeY anpassen für die verschobene Liste (Start bei y=105)
            local relativeY = my - 105 + State.listScroll
            local idx = math.floor(relativeY / 40) + 1
            if displayed[idx] then
                State.currentTileType = displayed[idx].id
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

            -- Malen mit Pinsel (brushRadius)
            local r = State.brushRadius or 0
            local minX = math.max(1, tx - r)
            local maxX = math.min(#(State.currentRoom[1] or {}), tx + r)
            local minY = math.max(1, ty - r)
            local maxY = math.min(#State.currentRoom, ty + r)

            for py = minY, maxY do
                for px = minX, maxX do
                    if State.currentRoom[py] and State.currentRoom[py][px] ~= nil then
                        -- Circle mask
                        local useTile = true
                        if (State.brushShape or "circle") == "circle" then
                            local dx = px - tx
                            local dy = py - ty
                            local dist = math.sqrt(dx*dx + dy*dy)
                            if dist > (r + 0.0001) then useTile = false end
                        end
                        if not useTile then goto continue_tile end

                        -- Falloff (probabilistic) oder harte Auswahl
                        local applyThis = true
                        if State.brushFalloff then
                            local dx = px - tx
                            local dy = py - ty
                            local dist = math.sqrt(dx*dx + dy*dy)
                            local dnorm = dist / (math.max(1, r))
                            if dnorm > 1 then applyThis = false end
                            if applyThis then
                                local strength = math.pow(math.max(0, 1 - dnorm), (State.brushFalloffExp or 1.0))
                                applyThis = (math.random() < strength)
                            end
                        end

                        if not applyThis then goto continue_tile end

                        if love.mouse.isDown(1) then
                            local before = State.currentRoom[py][px]
                            local after = State.currentTileType
                            if before ~= after then
                                if State.strokeActive then
                                    local found = false
                                    for _, e in ipairs(State.strokeBuffer) do
                                        if e.x == px and e.y == py then e.after = after; found = true; break end
                                    end
                                    if not found then table.insert(State.strokeBuffer, {x = px, y = py, before = before, after = after}) end
                                else
                                    Actions.recordTileChange(px, py, before, after)
                                end
                                State.currentRoom[py][px] = after
                            end
                        end

                        if love.mouse.isDown(2) then
                            local before = State.currentRoom[py][px]
                            local after = 0
                            if before ~= after then
                                if State.strokeActive then
                                    local found = false
                                    for _, e in ipairs(State.strokeBuffer) do
                                        if e.x == px and e.y == py then e.after = after; found = true; break end
                                    end
                                    if not found then table.insert(State.strokeBuffer, {x = px, y = py, before = before, after = after}) end
                                else
                                    Actions.recordTileChange(px, py, before, after)
                                end
                                State.currentRoom[py][px] = after
                            end
                        end
                    end
                    ::continue_tile::
                end
            end
        end
    end
end

function love.mousepressed(x, y, button)
    -- Behandelt Einmalklicks (Buttons, Auswahl, Weltplatzierung).
    -- Modal-Handling hat Vorrang (Save/Load Modals)
    if State.showLoadModal then
        local sw, sh = love.graphics.getDimensions()
        local hw, hh = 560, 300
        local hx = (sw - hw) / 2
        local hy = (sh - hh) / 2
        local listX, listY, listW, listH = hx + 18, hy + 36, hw - 36, hh - 100
        if x >= listX and x <= listX + listW and y >= listY and y <= listY + listH then
            local idx = math.floor((y - (listY + 4 - State.loadModalScroll)) / 22) + 1
            if State.worldFiles[idx] then
                State.loadModalSelected = State.worldFiles[idx].name
            end
            return
        end
    end

    if State.showHistoryModal then
        local sw, sh = love.graphics.getDimensions()
        local hw, hh = 640, 380
        local hx = (sw - hw) / 2
        local hy = (sh - hh) / 2
        local listX, listY, listW, listH = hx + 18, hy + 36, hw - 36, hh - 120
        if x >= listX and x <= listX + listW and y >= listY and y <= listY + listH then
            -- Build combined list length
            local undoCount = #State.undoStack
            local redoCount = #State.redoStack
            local idx = math.floor((y - (listY + 6 - State.historyScroll)) / 20) + 1
            local total = undoCount + redoCount
            if idx < 1 then idx = 1 end
            if idx > total then idx = total end
            State.historySelected = idx
            return
        end
    end
    if State.showSaveModal then
        local sw, sh = love.graphics.getDimensions()
        local hw, hh = 560, 160
        local hx = (sw - hw) / 2
        local hy = (sh - hh) / 2
        local inx, iny, inw, inh = hx + 18, hy + 36, hw - 36, 28
        if button == 1 and x >= inx and x <= inx + inw and y >= iny and y <= iny + inh then
            State.saveModalFocus = true
            return
        else
            State.saveModalFocus = false
        end
    end

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
        -- Klick in das Suchfeld der Tile-Liste?
        local searchX, searchY, searchW, searchH = 10, 70, UI_WIDTH - 20, 24
        if button == 1 and x >= searchX and x <= searchX + searchW and y >= searchY and y <= searchY + searchH then
            State.tileSearchFocus = true
            return
        end

        -- Klick in die Tile-Liste (außerhalb Suchfeld) => Auswahl
        if button == 1 and x < UI_WIDTH and y > 100 then
            State.tileSearchFocus = false
            local displayed = {}
            local filter = (State.tileSearch or ""):lower()
            for i, block in ipairs(Constants.BLOCK_TYPES) do
                if filter == "" or (block.name and block.name:lower():find(filter, 1, true)) then
                    table.insert(displayed, block)
                end
            end

            local relativeY = y - 105 + State.listScroll
            local idx = math.floor(relativeY / 40) + 1
            if displayed[idx] then
                State.currentTileType = displayed[idx].id
            end
        end

        -- Start einer Mal-Aktion (Stroke) wenn in der Map geklickt wurde
        if (button == 1 or button == 2) and x >= PANEL_X and y >= 80 then
            State.strokeActive = true
            State.strokeBuffer = {}
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

function love.mousereleased(x, y, button)
    -- Finalisiert aktive Painting-Strokes (zusammenfassen in Undo)
    if State.currentState == Constants.STATE.ROOM_EDIT then
        if State.strokeActive then
            if State.strokeBuffer and #State.strokeBuffer > 0 then
                Actions.recordStroke(State.strokeBuffer)
            end
            State.strokeActive = false
            State.strokeBuffer = {}
        end
    end
end

function love.keypressed(key)
    -- Tab wechselt Fokus zum Suchfeld
    if key == "tab" and State.currentState == Constants.STATE.ROOM_EDIT then
        State.tileSearchFocus = not State.tileSearchFocus
        State.saveModalFocus = false
        State.tileSearchCaretTimer = 0.5
        State.tileSearchCaretVisible = State.tileSearchFocus
        return
    end

    -- Eingabe-Fokus für Suchfeld / Modal (Backspace / Esc / Enter behandeln)
    if State.tileSearchFocus then
        if key == "backspace" then
            State.tileSearch = (State.tileSearch or ""):sub(1, -2)
            State.tileSearchCaretTimer = 0.5
            State.tileSearchCaretVisible = true
            return
        end
        if key == "escape" then
            State.tileSearchFocus = false
            return
        end
        if key == "return" or key == "kpenter" then
            -- Bestätige erste gefilterte Auswahl (falls vorhanden)
            local displayed = {}
            local filter = (State.tileSearch or ""):lower()
            for i, b in ipairs(Constants.BLOCK_TYPES) do
                if filter == "" or (b.name and b.name:lower():find(filter, 1, true)) then
                    table.insert(displayed, b)
                end
            end
            if #displayed > 0 then State.currentTileType = displayed[1].id end
            State.tileSearchFocus = false
            return
        end
    end
    if State.saveModalFocus then
        if key == "backspace" then
            State.saveModalFilename = (State.saveModalFilename or ""):sub(1, -2)
            return
        end
        if key == "escape" then
            State.showSaveModal = false
            State.saveModalFocus = false
            return
        end
    end

    -- Tastaturkürzel für Moduswechsel, Save/Load und Editor-Aktionen.
    if key == "f11" then
        love.window.setFullscreen(not love.window.getFullscreen())
    end

    if key == "h" or key == "?" then
        State.showHelp = not State.showHelp
        return
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

        -- Tile-Liste: Tastatur-Navigation (Up/Down) — respektiert aktiven Filter
        if key == "up" or key == "down" or key == "pageup" or key == "pagedown" or key == "home" or key == "end" then
            local displayed = {}
            local filter = (State.tileSearch or ""):lower()
            for i, b in ipairs(Constants.BLOCK_TYPES) do
                if filter == "" or (b.name and b.name:lower():find(filter, 1, true)) then
                    table.insert(displayed, b)
                end
            end
            if #displayed > 0 then
                -- Navigation (Up/Down) oder Scroll (Page/Home/End)
                if key == "up" or key == "down" then
                    local curIdx = 1
                    for i, b in ipairs(displayed) do if b.id == State.currentTileType then curIdx = i; break end end
                    if key == "up" then curIdx = math.max(1, curIdx - 1) else curIdx = math.min(#displayed, curIdx + 1) end
                    State.currentTileType = displayed[curIdx].id
                    State.listScroll = math.max(0, (curIdx - 4) * 40)
                else
                    local sh = love.graphics.getHeight()
                    local visibleH = math.max(40, sh - 180)
                    local itemH = 40
                    local maxScroll = math.max(0, (#displayed * itemH) - visibleH)
                    if key == "home" then State.listScroll = 0
                    elseif key == "end" then State.listScroll = maxScroll
                    elseif key == "pageup" then State.listScroll = math.max(0, State.listScroll - visibleH)
                    elseif key == "pagedown" then State.listScroll = math.min(maxScroll, State.listScroll + visibleH)
                    end
                end
            end
            return
        end

        -- Undo / Redo (Ctrl+Z / Ctrl+Y)
        if key == "z" and (love.keyboard.isDown("lctrl") or love.keyboard.isDown("rctrl")) then
            local ok, err = pcall(function() Actions.undo() end)
            return
        end
        if key == "y" and (love.keyboard.isDown("lctrl") or love.keyboard.isDown("rctrl")) then
            local ok, err = pcall(function() Actions.redo() end)
            return
        end

        -- History-Modal öffnen/schließen (U)
        if key == "u" then
            State.showHistoryModal = not State.showHistoryModal
            if State.showHistoryModal then State.historySelected = #State.undoStack end
            return
        end

        if key == "s" then
            -- Öffne internes Save-Modal
            State.showSaveModal = true
            State.saveModalFilename = State.currentFilename or "level"
            State.saveModalFocus = true
        elseif key == "l" then
            -- Öffne internes Load-Modal
            Actions.refreshWorldFileList()
            State.showLoadModal = true
            State.loadModalSelected = nil
        end

        -- Brush Größe ändern ( [ / ] )
        if key == "[" then
            State.brushRadius = math.max(0, (State.brushRadius or 0) - 1)
            Actions.setMessage("Brush: " .. (2 * State.brushRadius + 1) .. "x" .. (2 * State.brushRadius + 1), 1.0)
            return
        end
        if key == "]" then
            State.brushRadius = math.min(State.brushRadiusMax or 4, (State.brushRadius or 0) + 1)
            Actions.setMessage("Brush: " .. (2 * State.brushRadius + 1) .. "x" .. (2 * State.brushRadius + 1), 1.0)
            return
        end

        -- Brush Shape / Falloff: b = shape toggle, f = falloff toggle, , / . = falloff exponent
        if key == "b" then
            State.brushShape = (State.brushShape == "circle") and "square" or "circle"
            Actions.setMessage("Brush shape: " .. State.brushShape, 1.0)
            return
        end
        if key == "f" then
            State.brushFalloff = not State.brushFalloff
            Actions.setMessage("Brush falloff: " .. (State.brushFalloff and "on" or "off"), 1.0)
            return
        end
        if key == "," and State.brushFalloff then
            State.brushFalloffExp = math.max(0.2, (State.brushFalloffExp or 1.0) - 0.1)
            Actions.setMessage(string.format("Falloff: %.1f", State.brushFalloffExp), 1.0)
            return
        end
        if key == "." and State.brushFalloff then
            State.brushFalloffExp = math.min(4.0, (State.brushFalloffExp or 1.0) + 0.1)
            Actions.setMessage(string.format("Falloff: %.1f", State.brushFalloffExp), 1.0)
            return
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

-- Text input handler (für Suchfeld, Modals etc.)
function love.textinput(t)
    if State.tileSearchFocus then
        State.tileSearch = (State.tileSearch or "") .. t
        State.tileSearchCaretTimer = 0.5
        State.tileSearchCaretVisible = true
        return
    end
    if State.saveModalFocus then
        State.saveModalFilename = (State.saveModalFilename or "") .. t
        return
    end
end

function love.wheelmoved(x, y)
    -- Mausrad für Listen-Scroll und Zoomsteuerung.
    local mx, my = love.mouse.getPosition()

    -- Falls Load-Modal offen ist, scrolle dort
    if State.showLoadModal then
        State.loadModalScroll = math.max(0, State.loadModalScroll - y * 30)
        return
    end

    -- Falls History-Modal offen ist, scrolle dort
    if State.showHistoryModal then
        State.historyScroll = math.max(0, State.historyScroll - y * 30)
        return
    end

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
