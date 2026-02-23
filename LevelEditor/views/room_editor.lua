local State = require "state"
local Constants = require "constants"
local UI = require "ui_components"
local Actions = require "editor_actions"
local IOUtils = require "io_utils"

local M = {}

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

function M.drawRoomEditor()
    local rightPanelX = 220
    local uiW = 200
    local sw, sh = love.graphics.getDimensions()

    -- Sidebar Hintergrund
    love.graphics.setColor(0.15, 0.15, 0.15)
    love.graphics.rectangle("fill", 0, 0, uiW, sh)

    -- Zurück-Button oben rechts statt links
    local backW, backH = 100, 28
    if UI.drawButton("Zurück", sw - backW - 10, 10, backW, backH) then
        State.currentState = Constants.STATE.MENU
    end

    -- Header / Toolbar
    love.graphics.setColor(1, 1, 1)
    -- Toolbar (Icons) bleibt oben, Header wird unterhalb gezeichnet und bei Bedarf gekürzt
    -- Glyphs replaced with simple ASCII letters for compatibility
    if UI.drawButton("S", rightPanelX + 10, 10, 34, 28) then
        State.showSaveModal = true
        State.saveModalFilename = State.currentFilename or "level"
    end
    if UI.drawButton("U", rightPanelX + 50, 10, 34, 28) then Actions.undo() end
    if UI.drawButton("R", rightPanelX + 90, 10, 34, 28) then Actions.redo() end
    if UI.drawButton("L", rightPanelX + 130, 10, 34, 28) then Actions.refreshWorldFileList(); State.showLoadModal = true end
    if UI.drawButton("H", rightPanelX + 170, 10, 34, 28) then State.showHistoryModal = true; State.historySelected = #State.undoStack end
    if UI.drawButton("+", rightPanelX + 210, 10, 28, 28) then State.zoom = math.min(3, State.zoom + 0.1) end
    if UI.drawButton("-", rightPanelX + 248, 10, 28, 28) then State.zoom = math.max(0.25, State.zoom - 0.1) end
    if UI.drawButton("?", rightPanelX + 286, 10, 28, 28) then State.showHelp = not State.showHelp end

    -- Kleine Labels unter den Icons
    love.graphics.setColor(1,1,1)
    love.graphics.print("Speichern", rightPanelX + 6, 40)
    love.graphics.print("Undo", rightPanelX + 46, 40)
    love.graphics.print("Redo", rightPanelX + 86, 40)
    love.graphics.print("Laden", rightPanelX + 126, 40)
    love.graphics.print("History", rightPanelX + 170, 40)
    love.graphics.print("Zoom", rightPanelX + 218, 40)
    love.graphics.print("Hilfe", rightPanelX + 284, 40)

    -- Header (unterhalb der Toolbar) — bei Bedarf kürzen, damit keine Überlappung entsteht
    local headerText = "Raum: " .. UI.sanitizeForDisplay(State.currentFilename) .. " (" .. State.mapScreensW .. "x" .. State.mapScreensH .. ")"
    local headerMaxW = sw - rightPanelX - 20
    love.graphics.print(UI.truncate(headerText, headerMaxW), rightPanelX + 10, 64)

    -- Kurze Tastatur-Hinweise (unter Header, wrapped)
    love.graphics.setColor(0.85,0.85,0.85)
    love.graphics.printf("[S] Speichern | [L] Laden | [ESC] Menü | [F11] Vollbild | [H] Hilfe | [ / ] Brush", rightPanelX + 10, 86, sw - rightPanelX - 20)
    love.graphics.setColor(1,1,1)

    -- Brush Anzeige
    local brushDisplay = (2 * (State.brushRadius or 0) + 1) .. "x" .. (2 * (State.brushRadius or 0) + 1)
    love.graphics.setColor(0.9, 0.9, 0.6)
    love.graphics.print("Brush: " .. brushDisplay .. "  [Shape: " .. (State.brushShape or "circle") .. "]  [Falloff: " .. (State.brushFalloff and (string.format("on %.1f", State.brushFalloffExp)) or "off") .. "]", rightPanelX + 10, 106)
    love.graphics.setColor(1,1,1)

    if State.message ~= "" then
        love.graphics.setColor(0, 1, 0)
        love.graphics.print(UI.sanitizeForDisplay(State.message), rightPanelX + 10, 50)
        love.graphics.setColor(1, 1, 1)
    end

    -- Tooltip (bei Button-Hover)
    if UI._lastHover then
        local mx, my = love.mouse.getPosition()
        local tw = love.graphics.getFont():getWidth(UI._lastHover)
        love.graphics.setColor(0,0,0,0.8)
        love.graphics.rectangle("fill", mx + 12, my + 12, tw + 12, 20)
        love.graphics.setColor(1,1,1)
        love.graphics.print(UI._lastHover, mx + 18, my + 14)
    end

    -- Sidebar Hintergrund
    love.graphics.setColor(0.15, 0.15, 0.15)
    love.graphics.rectangle("fill", 0, 0, uiW, sh)
    love.graphics.setColor(1, 1, 1)
    love.graphics.rectangle("line", 0, 0, uiW, sh)

        local toolLabel = (State.currentTool == "item") and "Items:" or "Tiles:"
    love.graphics.print(toolLabel, 10, 45)

    -- Suchfeld für Tile-Liste
    local searchX, searchY, searchW, searchH = 10, 70, uiW - 20, 24
    love.graphics.setColor(0.1, 0.1, 0.1)
    love.graphics.rectangle("fill", searchX, searchY, searchW, searchH)
    love.graphics.setColor(1,1,1)
    love.graphics.rectangle("line", searchX, searchY, searchW, searchH)
    local displaySearch = State.tileSearch or ""
    -- Focus-Outline
    if State.tileSearchFocus then
        love.graphics.setColor(1,1,1)
        love.graphics.rectangle("line", searchX - 1, searchY - 1, searchW + 2, searchH + 2)
    end
    love.graphics.print(displaySearch == "" and "Suche..." or displaySearch, searchX + 6, searchY + 4)

    -- Caret (blinkend)
    if State.tileSearchFocus and State.tileSearchCaretVisible then
        local font = love.graphics.getFont()
        local txt = displaySearch or ""
        local tx = searchX + 6 + font:getWidth(txt)
        local caretX = math.min(tx, searchX + searchW - 8)
        love.graphics.setColor(1,1,1)
        love.graphics.rectangle("fill", caretX, searchY + 6, 1, font:getHeight() - 8)
    end

    -- Clear-Button im Suchfeld
    if (State.tileSearch or "") ~= "" then
        if UI.drawButton("✕", searchX + searchW - 22, searchY + 2, 20, 20) then
            State.tileSearch = ""
            State.listScroll = 0
            State.tileSearchCaretTimer = 0.5
            State.tileSearchCaretVisible = true
        end
    end

    -- Liste der gefilterten Blöcke
    love.graphics.setScissor(0, 100, uiW, sh - 180)
        local startY = 105 - State.listScroll
        local displayed = {}
        local filter = (State.tileSearch or ""):lower()

        local list = (State.currentTool == "item") and Constants.ITEM_TYPES or Constants.BLOCK_TYPES
        for i, block in ipairs(list) do
            local name = block.name or ""
            if filter == "" or name:lower():find(filter, 1, true) then
                table.insert(displayed, block)
            end
        end

        for i, block in ipairs(displayed) do
            local y = startY + (i-1) * 40

            -- Ausgewählt Highlight (abhängig vom aktuellen Tool)
            local selId = State.currentTool == "item" and Constants.ITEM_TYPES[State.currentItemType].id or State.currentTileType
            if selId == block.id then
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

    -- Cursor / Brush Overlay (World space)
    do
        local mx, my = love.mouse.getPosition()
        local localX = mx - rightPanelX
        local localY = my - 80
        local worldMX = (localX / State.zoom) + State.camX - mapOffsetX
        local worldMY = (localY / State.zoom) + State.camY - mapOffsetY
        local cursorTx = math.floor(worldMX / Constants.TILE_SIZE) + 1
        local cursorTy = math.floor(worldMY / Constants.TILE_SIZE) + 1
        local r = State.brushRadius or 0
        local bx1 = mapOffsetX + (cursorTx - 1 - r) * Constants.TILE_SIZE
        local by1 = mapOffsetY + (cursorTy - 1 - r) * Constants.TILE_SIZE
        local bw = (2*r + 1) * Constants.TILE_SIZE
        local bh = (2*r + 1) * Constants.TILE_SIZE
        if cursorTx >= 1 and cursorTy >= 1 and cursorTx <= (#(State.currentRoom[1] or {})) and cursorTy <= #State.currentRoom then
            if State.brushFalloff then
                -- Per-Tile alpha based on distance to center
                for py = cursorTy - r, cursorTy + r do
                    for px = cursorTx - r, cursorTx + r do
                        local tileX = mapOffsetX + (px-1) * Constants.TILE_SIZE
                        local tileY = mapOffsetY + (py-1) * Constants.TILE_SIZE
                        local dx = px - cursorTx
                        local dy = py - cursorTy
                        local dist = math.sqrt(dx*dx + dy*dy)
                        if dist <= r + 0.0001 then
                            local dnorm = dist / math.max(1, r)
                            local strength = math.pow(math.max(0, 1 - dnorm), (State.brushFalloffExp or 1.0))
                            local alpha = 0.12 * strength
                            love.graphics.setColor(1,1,1, alpha)
                            love.graphics.rectangle("fill", tileX, tileY, Constants.TILE_SIZE, Constants.TILE_SIZE)
                        end
                    end
                end
                love.graphics.setColor(1,1,1,0.6)
                love.graphics.rectangle("line", bx1, by1, bw, bh)
                love.graphics.setColor(1,1,1)
            else
                love.graphics.setColor(1,1,1,0.12)
                love.graphics.rectangle("fill", bx1, by1, bw, bh)
                love.graphics.setColor(1,1,1,0.6)
                love.graphics.rectangle("line", bx1, by1, bw, bh)
                love.graphics.setColor(1,1,1)
            end
        end
    end
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

    -- Items rendern (blaue Platzhalter)
    love.graphics.setColor(0,0,1)
    for _, it in ipairs(State.currentRoomItems) do
        local drawX = mapOffsetX + (it.x-1) * Constants.TILE_SIZE
        local drawY = mapOffsetY + (it.y-1) * Constants.TILE_SIZE
        love.graphics.rectangle("fill", drawX, drawY, Constants.TILE_SIZE, Constants.TILE_SIZE)
    end

    love.graphics.setColor(1,1,1)
    love.graphics.pop()
    love.graphics.setScissor()

    -- Save / Load Modals (in-editor)
    if State.showSaveModal then
        local hw, hh = 560, 160
        local hx = (sw - hw) / 2
        local hy = (sh - hh) / 2
        love.graphics.setColor(0, 0, 0, 0.9)
        love.graphics.rectangle("fill", hx, hy, hw, hh, 8, 8)
        love.graphics.setColor(1, 1, 1)
        love.graphics.rectangle("line", hx, hy, hw, hh, 8, 8)

        love.graphics.print("Speichern unter (Dateiname):", hx + 18, hy + 12)
        local inx, iny, inw, inh = hx + 18, hy + 36, hw - 36, 28
        love.graphics.setColor(0.1, 0.1, 0.1)
        love.graphics.rectangle("fill", inx, iny, inw, inh)
        love.graphics.setColor(1,1,1)
        love.graphics.rectangle("line", inx, iny, inw, inh)
        love.graphics.print(State.saveModalFilename or "", inx + 6, iny + 6)

        -- Buttons
        if UI.drawButton("Native Dialog", hx + hw - 430, hy + hh - 46, 120, 32) then
            local path = IOUtils.showSaveDialog()
            if path then Actions.saveRoom(path); State.showSaveModal = false; State.saveModalFocus = false end
        end
        if UI.drawButton("Speichern", hx + hw - 280, hy + hh - 46, 100, 32) then
            local filename = State.saveModalFilename or ""
            if filename == "" then Actions.setMessage("Kein Dateiname eingegeben", 1.5)
            else
                local path = IOUtils.getProjectDir() .. "\\level\\" .. filename .. ".lua"
                local exists = IOUtils.readFilePS(path)
                if exists and not State.saveModalConfirmOverwrite then
                    State.saveModalConfirmOverwrite = true
                else
                    Actions.saveRoom(path)
                    State.currentFilename = filename
                    State.showSaveModal = false
                    State.saveModalFocus = false
                    State.saveModalConfirmOverwrite = false
                end
            end
        end
        if UI.drawButton("Abbrechen", hx + hw - 170, hy + hh - 46, 100, 32) then
            State.showSaveModal = false
            State.saveModalFocus = false
            State.saveModalConfirmOverwrite = false
        end

        if State.saveModalConfirmOverwrite then
            love.graphics.setColor(1, 0.8, 0.2)
            love.graphics.print("Datei existiert - überschreiben?", hx + 18, hy + 76)
            if UI.drawButton("Überschreiben", hx + hw - 430, hy + hh - 86, 120, 28) then
                local filename = State.saveModalFilename or ""
                local path = IOUtils.getProjectDir() .. "\\level\\" .. filename .. ".lua"
                Actions.saveRoom(path)
                State.showSaveModal = false
                State.saveModalFocus = false
                State.saveModalConfirmOverwrite = false
            end
            if UI.drawButton("Abbrechen", hx + hw - 290, hy + hh - 86, 120, 28) then
                State.saveModalConfirmOverwrite = false
            end
        end
    end

    if State.showLoadModal then
        local hw, hh = 560, 300
        local hx = (sw - hw) / 2
        local hy = (sh - hh) / 2
        love.graphics.setColor(0, 0, 0, 0.9)
        love.graphics.rectangle("fill", hx, hy, hw, hh, 8, 8)
        love.graphics.setColor(1, 1, 1)
        love.graphics.rectangle("line", hx, hy, hw, hh, 8, 8)

        love.graphics.print("Datei öffnen:", hx + 18, hy + 12)
        -- Datei-Liste (State.worldFiles wird hier wiederverwendet)
        local listX, listY, listW, listH = hx + 18, hy + 36, hw - 36, hh - 100
        love.graphics.setColor(0.08, 0.08, 0.08)
        love.graphics.rectangle("fill", listX, listY, listW, listH)
        love.graphics.setColor(1,1,1)
        love.graphics.rectangle("line", listX, listY, listW, listH)

        love.graphics.setScissor(listX, listY, listW, listH)
        local startY = listY + 4 - State.loadModalScroll
        for i, fileData in ipairs(State.worldFiles) do
            local y = startY + (i-1) * 22
            if State.loadModalSelected == fileData.name then
                love.graphics.setColor(0.3,0.3,0.3)
                love.graphics.rectangle("fill", listX + 2, y - 2, listW - 4, 20)
                love.graphics.setColor(1,1,0)
                love.graphics.rectangle("line", listX + 2, y - 2, listW - 4, 20)
            end
            love.graphics.setColor(1,1,1)
            love.graphics.print(fileData.name, listX + 6, y)
        end
        love.graphics.setScissor()

        if UI.drawButton("Laden", hx + hw - 170, hy + hh - 48, 100, 32) then
            if State.loadModalSelected then
                local path = IOUtils.getProjectDir() .. "\\level\\" .. State.loadModalSelected .. ".lua"
                Actions.loadRoomFromPath(path)
                State.showLoadModal = false
                State.loadModalSelected = nil
            else
                Actions.setMessage("Keine Datei ausgewählt", 1.5)
            end
        end
        if UI.drawButton("Native Dialog", hx + hw - 300, hy + hh - 48, 120, 32) then
            local path = IOUtils.showOpenDialog()
            if path then Actions.loadRoomFromPath(path); State.showLoadModal = false end
        end
        if UI.drawButton("Abbrechen", hx + hw - 80, hy + hh - 48, 60, 32) then
            State.showLoadModal = false
            State.loadModalSelected = nil
        end
    end

    -- Undo History Modal
    if State.showHistoryModal then
        local hw, hh = 640, 380
        local hx = (sw - hw) / 2
        local hy = (sh - hh) / 2
        love.graphics.setColor(0,0,0,0.95)
        love.graphics.rectangle("fill", hx, hy, hw, hh, 8, 8)
        love.graphics.setColor(1,1,1)
        love.graphics.rectangle("line", hx, hy, hw, hh, 8, 8)

        love.graphics.print("Undo‑History", hx + 18, hy + 12)
        local listX, listY, listW, listH = hx + 18, hy + 36, hw - 36, hh - 120
        love.graphics.setColor(0.06, 0.06, 0.06)
        love.graphics.rectangle("fill", listX, listY, listW, listH)
        love.graphics.setColor(1,1,1)
        love.graphics.rectangle("line", listX, listY, listW, listH)

        -- Build combined list (undoStack oldest->newest, then redoStack newest->oldest)
        local combined = {}
        for i=1, #State.undoStack do table.insert(combined, {entry = State.undoStack[i], kind = "undo"}) end
        for i = #State.redoStack, 1, -1 do table.insert(combined, {entry = State.redoStack[i], kind = "redo"}) end

        love.graphics.setScissor(listX, listY, listW, listH)
        local startY = listY + 6 - State.historyScroll
        for i, item in ipairs(combined) do
            local y = startY + (i-1) * 20
            if State.historySelected == i then
                love.graphics.setColor(0.25,0.25,0.25)
                love.graphics.rectangle("fill", listX + 2, y - 2, listW - 4, 18)
                love.graphics.setColor(1,1,0)
                love.graphics.rectangle("line", listX + 2, y - 2, listW - 4, 18)
            end
            love.graphics.setColor(item.kind == "redo" and 0.7 or 0.9, item.kind == "redo" and 0.7 or 0.9, item.kind == "redo" and 0.7 or 0.9)
            love.graphics.print((item.kind == "undo") and ("[U] " .. Actions.describeHistoryEntry(item.entry)) or ("[R] " .. Actions.describeHistoryEntry(item.entry)), listX + 6, y)
        end
        love.graphics.setScissor()

        -- Buttons
        if UI.drawButton("Revert to here", hx + hw - 360, hy + hh - 64, 140, 32) then
            if State.historySelected then
                local target = State.historySelected
                -- targetUndoCount is number of undo entries to remain
                local undoCount = #State.undoStack
                if target <= undoCount then
                    Actions.jumpToHistory(target)
                else
                    Actions.jumpToHistory(target)
                end
                State.showHistoryModal = false
                State.historySelected = nil
            else
                Actions.setMessage("Keine Aktion ausgewählt", 1.2)
            end
        end
        if UI.drawButton("Clear History", hx + hw - 200, hy + hh - 64, 120, 32) then
            Actions.clearHistory()
            State.showHistoryModal = false
            State.historySelected = nil
        end
        if UI.drawButton("Abbrechen", hx + hw - 80, hy + hh - 64, 60, 32) then
            State.showHistoryModal = false
            State.historySelected = nil
        end
    end

    -- Hilfe Overlay (zentral, modal)
    if State.showHelp then
        local hw, hh = 640, 260
        local hx = (sw - hw) / 2
        local hy = (sh - hh) / 2
        love.graphics.setColor(0, 0, 0, 0.85)
        love.graphics.rectangle("fill", hx, hy, hw, hh, 8, 8)
        love.graphics.setColor(1, 1, 1)
        love.graphics.rectangle("line", hx, hy, hw, hh, 8, 8)

        local ox = hx + 20
        local oy = hy + 18
        local helpText = "Kurz-Anleitung — Raum-Editor\n\n• Linke Maustaste: malen    • Rechte Maustaste: löschen\n• 1/2/3: Tile-Auswahl       • Pfeiltasten: Kamera\n• Mausrad: Liste scrollen / Karte zoomen\n• [S]: Speichern   • [L]: Laden   • [ESC]: Menü\n• Buttons oben: Save / Load / Zoom / Hilfe (H drücken)"
        love.graphics.printf(helpText, ox, oy, hw - 40)

        if UI.drawButton("Verstanden", hx + hw - 120, hy + hh - 48, 100, 32) then
            State.showHelp = false
        end
    end
end

return M
