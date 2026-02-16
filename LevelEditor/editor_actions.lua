local State = require "state"
local Constants = require "constants"
local IOUtils = require "io_utils"

local Actions = {}

-- Heuristiken für Stroke-Merging (Zeit in Sekunden, Abstand in Tiles)
local MERGE_TIME = 0.8
local MERGE_MARGIN = 2

-- Hilfsfunktion: merge newChanges in targetChanges (dedupliziert nach x/y, neuere 'after' gewinnt)
local function mergeChanges(targetChanges, newChanges)
    for _, nc in ipairs(newChanges) do
        local found = false
        for _, tc in ipairs(targetChanges) do
            if tc.x == nc.x and tc.y == nc.y then
                tc.after = nc.after
                found = true
                break
            end
        end
        if not found then table.insert(targetChanges, {x = nc.x, y = nc.y, before = nc.before, after = nc.after}) end
    end
end

-- Erstellt eine neue, leere Map mit den angegebenen Maßen (in Screens)
function Actions.createMap(w, h)
    State.mapScreensW = w
    State.mapScreensH = h
    State.currentRoom = {}

    local totalTilesX = w * Constants.TILES_X
    local totalTilesY = h * Constants.TILES_Y

    for y=1, totalTilesY do
        State.currentRoom[y] = {}
        for x=1, totalTilesX do
            State.currentRoom[y][x] = 0 -- 0 = Luft

            -- Optional: Automatischen Rahmen (Wände) um die Map ziehen
            if x==1 or x==totalTilesX or y==1 or y==totalTilesY then
                State.currentRoom[y][x] = 1 -- Wand
            end
        end
    end
end

-- Speichert den aktuellen Raum in eine Datei
function Actions.saveRoom(filename)
    -- Lua Table als String serialisieren
    local content = "return {\n"

    -- Grid speichern
    content = content .. "  grid = {\n"
    for y, row in ipairs(State.currentRoom) do
        content = content .. "  {"
        for x, tile in ipairs(row) do
            content = content .. tile .. ","
        end
        content = content .. "},\n"
    end
    content = content .. "  }\n"

    content = content .. "}"

    IOUtils.writeFilePS(filename, content)
end

-- Durchsucht das Verzeichnis nach Level-Dateien für den Welt-Editor
function Actions.refreshWorldFileList()
    State.worldFiles = {}

    local dir = IOUtils.getProjectDir() .. "\\level"
    local dirB64 = IOUtils.toBase64(dir)
    -- PowerShell "ls" Equivalent
    local cmd = 'powershell -command "$path = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String(\'' .. dirB64 .. '\')); Get-ChildItem -Path $path -Filter *.lua | Select-Object -ExpandProperty Name"'

    local handle = io.popen(cmd)
    if handle then
        local result = handle:read("*a")
        handle:close()
        for line in result:gmatch("[^\r\n]+") do
            if line ~= "" and line ~= "world.lua" then -- world.lua selbst ausschließen
                local name = line:sub(1, -5) -- .lua Endung entfernen

                -- Metadaten (Größe) laden, falls noch nicht im Cache
                if not State.roomCache[name] then
                   local path = dir .. "\\" .. line
                   local content = IOUtils.readFilePS(path)
                   if content then
                       local chunk = load(content)
                       if chunk then
                           local data = chunk()
                           if data then
                               local grid = data.grid or data

                               local h = #grid
                               local w = 0
                               if h > 0 then w = #grid[1] end

                               -- Maße in Screens umrechnen
                               local screensH = 1
                               local screensW = 1
                               if Constants.TILES_Y > 0 then screensH = math.max(1, math.ceil(h / Constants.TILES_Y)) end
                               if Constants.TILES_X > 0 then screensW = math.max(1, math.ceil(w / Constants.TILES_X)) end

                               State.roomCache[name] = {w = screensW, h = screensH}
                           end
                       end
                   end
                end

                local meta = State.roomCache[name] or {w=1, h=1}
                table.insert(State.worldFiles, {name = name, w = meta.w, h = meta.h})
            end
        end
    end
end

-- Speichert das Welt-Grid
function Actions.saveWorld()
    local content = "return {\n"

    content = content .. "  grid = {\n"
    for k, v in pairs(State.worldGrid) do
        content = content .. "    ['" .. k .. "'] = '" .. v .. "',\n"
    end
    content = content .. "  },\n"

    content = content .. "  checkpoints = {\n"
    for k, v in pairs(State.worldCheckpoints) do
        if v then
            content = content .. "    ['" .. k .. "'] = true,\n"
        end
    end
    content = content .. "  }\n"

    content = content .. "}"

    local path = IOUtils.getProjectDir() .. "\\level\\world.lua"
    IOUtils.writeFilePS(path, content)

    Actions.setMessage("Welt gespeichert!", 3)
end

-- Lädt das Welt-Grid
function Actions.loadWorld()
    local path = IOUtils.getProjectDir() .. "\\level\\world.lua"
    local content = IOUtils.readFilePS(path)
    if content then
       local chunk = load(content)
       if chunk then
           local data = chunk()
           if data.grid then
               State.worldGrid = data.grid
               State.worldCheckpoints = data.checkpoints or {}
           else
               State.worldGrid = data -- Support für altes Format
               State.worldCheckpoints = {}
           end
           Actions.setMessage("Welt geladen!", 3)
       end
    end
end

-- Tile-Änderung in die Undo-Historie aufnehmen
function Actions.recordTileChange(tx, ty, before, after)
    if before == after then return end
    local now = (love and love.timer and love.timer.getTime) and love.timer.getTime() or os.clock()

    -- Versuche Merge in letzten Stroke
    local last = State.undoStack[#State.undoStack]
    if last and last.type == "stroke" and last.timestamp and (now - last.timestamp) <= MERGE_TIME then
        -- berechne BoundingBox von last
        local minx, miny, maxx, maxy = math.huge, math.huge, -math.huge, -math.huge
        for _, e in ipairs(last.changes) do
            minx = math.min(minx, e.x); miny = math.min(miny, e.y)
            maxx = math.max(maxx, e.x); maxy = math.max(maxy, e.y)
        end
        if tx >= (minx - MERGE_MARGIN) and tx <= (maxx + MERGE_MARGIN) and ty >= (miny - MERGE_MARGIN) and ty <= (maxy + MERGE_MARGIN) then
            -- merge into existing stroke
            local merged = false
            for _, e in ipairs(last.changes) do
                if e.x == tx and e.y == ty then e.after = after; merged = true; break end
            end
            if not merged then table.insert(last.changes, {x = tx, y = ty, before = before, after = after}) end
            last.timestamp = now
            State.redoStack = {}
            return
        end
    end

    -- Falls letzter Eintrag ein einzelnes Tile ist und nahe/zeitlich passt -> konvertiere zu Stroke
    if last and last.type == "tile" and last.timestamp and (now - last.timestamp) <= MERGE_TIME then
        if math.abs(last.x - tx) + math.abs(last.y - ty) <= MERGE_MARGIN then
            -- Ersetze letzten Eintrag mit Stroke, bestehende + neue Änderung
            local stroke = {type = "stroke", changes = {{x = last.x, y = last.y, before = last.before, after = last.after}, {x = tx, y = ty, before = before, after = after}}, timestamp = now}
            State.undoStack[#State.undoStack] = stroke
            State.redoStack = {}
            return
        end
    end

    -- Default: neuer Tile-Eintrag (mit timestamp)
    table.insert(State.undoStack, {type = "tile", x = tx, y = ty, before = before, after = after, timestamp = now})
    while #State.undoStack > State.maxHistory do table.remove(State.undoStack, 1) end
    State.redoStack = {}
end

-- Stroke (mehrere Tile-Änderungen als eine Aktion) aufzeichnen
function Actions.recordStroke(changes)
    if not changes or #changes == 0 then return end
    local now = (love and love.timer and love.timer.getTime) and love.timer.getTime() or os.clock()

    -- Merge mit letztem Stroke, falls zeitlich & räumlich nahe
    local last = State.undoStack[#State.undoStack]
    if last and last.type == "stroke" and last.timestamp and (now - last.timestamp) <= MERGE_TIME then
        -- Bounding Box Prüfung
        local minx, miny, maxx, maxy = math.huge, math.huge, -math.huge, -math.huge
        for _, e in ipairs(last.changes) do
            minx = math.min(minx, e.x); miny = math.min(miny, e.y)
            maxx = math.max(maxx, e.x); maxy = math.max(maxy, e.y)
        end
        -- prüfe, ob irgendeine neue Änderung in den erweiterten Bereich fällt
        local intersects = false
        for _, nc in ipairs(changes) do
            if nc.x >= (minx - MERGE_MARGIN) and nc.x <= (maxx + MERGE_MARGIN) and nc.y >= (miny - MERGE_MARGIN) and nc.y <= (maxy + MERGE_MARGIN) then
                intersects = true; break
            end
        end
        if intersects then
            mergeChanges(last.changes, changes)
            last.timestamp = now
            State.redoStack = {}
            return
        end
    end

    -- Sonst neuen Stroke eintragen (mit Timestamp)
    table.insert(State.undoStack, {type = "stroke", changes = changes, timestamp = now})
    while #State.undoStack > State.maxHistory do table.remove(State.undoStack, 1) end
    State.redoStack = {}
end

-- Rückgängig (Undo) der letzten Aktion
function Actions.undo()
    local act = table.remove(State.undoStack)
    if not act then Actions.setMessage("Nichts zu rückgängig", 1.2); return end

    if act.type == "tile" then
        if State.currentRoom[act.y] and State.currentRoom[act.y][act.x] then
            -- Zustand zurücksetzen und inversen Eintrag für Redo anlegen
            local cur = State.currentRoom[act.y][act.x]
            State.currentRoom[act.y][act.x] = act.before
            table.insert(State.redoStack, {type = "tile", x = act.x, y = act.y, before = act.after, after = act.before})
            Actions.setMessage("Undo", 0.8)
        end
    elseif act.type == "stroke" then
        -- Rückgängig: alle Änderungen der Stroke in umgekehrter Reihenfolge anwenden
        for i = #act.changes, 1, -1 do
            local e = act.changes[i]
            if State.currentRoom[e.y] and State.currentRoom[e.y][e.x] then
                State.currentRoom[e.y][e.x] = e.before
            end
        end
        -- Redo-Eintrag erzeugen (invertierte Änderungen)
        local inv = {type = "stroke", changes = {}}
        for i, e in ipairs(act.changes) do
            table.insert(inv.changes, {x = e.x, y = e.y, before = e.after, after = e.before})
        end
        table.insert(State.redoStack, inv)
        Actions.setMessage("Undo (Stroke)", 0.8)
    end
end

-- Wiederherstellen (Redo)
function Actions.redo()
    local act = table.remove(State.redoStack)
    if not act then Actions.setMessage("Nichts zu wiederholen", 1.2); return end

    if act.type == "tile" then
        if State.currentRoom[act.y] and State.currentRoom[act.y][act.x] then
            State.currentRoom[act.y][act.x] = act.after
            table.insert(State.undoStack, {type = "tile", x = act.x, y = act.y, before = act.before, after = act.after})
            Actions.setMessage("Redo", 0.8)
        end
    elseif act.type == "stroke" then
        -- Stroke Redo: alle Änderungen anwenden
        for i, e in ipairs(act.changes) do
            if State.currentRoom[e.y] and State.currentRoom[e.y][e.x] then
                State.currentRoom[e.y][e.x] = e.after
            end
        end
        table.insert(State.undoStack, act)
        Actions.setMessage("Redo (Stroke)", 0.8)
    end
end

-- Lädt einen Raum direkt aus einem Pfad (wiederverwendbar)
function Actions.loadRoomFromPath(path)
    local content = IOUtils.readFilePS(path)
    if not content then return false, "Datei konnte nicht gelesen werden" end
    local chunk, err = load(content)
    if not chunk then return false, err end
    local result = chunk()
    if result then
        if result.grid then State.currentRoom = result.grid else State.currentRoom = result end
        State.currentFilename = path:match("([^\\]+)$") or path
        Actions.setMessage("Geladen!", 2)
        return true
    end
    return false, "Ungültiges Format"
end

-- Beschreibe einen Undo/Redo-Eintrag für UI
function Actions.describeHistoryEntry(entry)
    if not entry then return "(leer)" end
    if entry.type == "tile" then
        local function tileName(id)
            if not id or id == 0 then return "(leer)" end
            for _, b in ipairs(Constants.BLOCK_TYPES) do if b.id == id then return b.name end end
            return tostring(id)
        end
        return string.format("Tile (%d,%d): %s → %s", entry.x or 0, entry.y or 0, tileName(entry.before), tileName(entry.after))
    elseif entry.type == "stroke" then
        return string.format("Stroke (%d Änderungen)", #entry.changes)
    else
        return entry.type or "Aktion"
    end
end

-- Springe in die History auf eine bestimmte Anzahl von Undo-Einträgen (0 == Anfang)
function Actions.jumpToHistory(targetUndoCount)
    local target = math.max(0, math.floor(targetUndoCount or 0))
    local cur = #State.undoStack

    -- Gültigkeitsbereich prüfen (kann nicht weiter als redoStack erlauben)
    local maxPossible = cur + #State.redoStack
    if target > maxPossible then target = maxPossible end

    while #State.undoStack > target do
        Actions.undo()
    end
    while #State.undoStack < target do
        Actions.redo()
    end
    Actions.setMessage("History: Sprung durchgeführt", 1.2)
end

function Actions.clearHistory()
    State.undoStack = {}
    State.redoStack = {}
    Actions.setMessage("History geleert", 1.2)
end

-- Hilfsfunktion für Nachrichten
function Actions.setMessage(msg, time)
    State.message = msg
    State.messageTimer = time
end

return Actions
