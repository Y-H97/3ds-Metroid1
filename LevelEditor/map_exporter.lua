local MapExporter = {}

-- Prüft, ob eine Lua-Tabelle ein dichtes 1..N-Array ist.
local function isArray(tbl)
    if type(tbl) ~= "table" then return false end
    local count = 0
    for key, _ in pairs(tbl) do
        if type(key) ~= "number" or key < 1 or key ~= math.floor(key) then
            return false
        end
        count = count + 1
    end
    for index = 1, count do
        if tbl[index] == nil then return false end
    end
    return true
end

local function escapeJsonString(value)
    -- Escaped Sonderzeichen für gültigen JSON-String.
    value = value:gsub("\\", "\\\\")
    value = value:gsub('"', '\\"')
    value = value:gsub("\n", "\\n")
    value = value:gsub("\r", "\\r")
    value = value:gsub("\t", "\\t")
    return value
end

local function jsonEncode(value)
    -- Minimaler JSON-Encoder für die benötigten Datentypen.
    local t = type(value)
    if t == "nil" then
        return "null"
    elseif t == "boolean" then
        return value and "true" or "false"
    elseif t == "number" then
        return tostring(value)
    elseif t == "string" then
        return '"' .. escapeJsonString(value) .. '"'
    elseif t == "table" then
        if isArray(value) then
            local parts = {}
            for i = 1, #value do
                parts[#parts + 1] = jsonEncode(value[i])
            end
            return "[" .. table.concat(parts, ", ") .. "]"
        else
            local parts = {}
            for key, item in pairs(value) do
                parts[#parts + 1] = '"' .. escapeJsonString(tostring(key)) .. '": ' .. jsonEncode(item)
            end
            table.sort(parts)
            return "{" .. table.concat(parts, ", ") .. "}"
        end
    else
        error("JSON encode: unsupported type " .. t)
    end
end

local function readLuaTable(path)
    -- Lädt eine Lua-Datei und führt sie sicher via pcall aus.
    local f, err = io.open(path, "rb")
    if not f then return nil, err end
    local content = f:read("*a")
    f:close()

    local chunk, loadErr = load(content, "@" .. path)
    if not chunk then return nil, loadErr end

    local ok, data = pcall(chunk)
    if not ok then return nil, data end
    return data
end

local function writeText(path, content)
    -- Schreibt Textdatei vollständig (UTF-8 als Raw-Bytes).
    local f, err = io.open(path, "wb")
    if not f then return false, err end
    f:write(content)
    f:close()
    return true
end

local function flattenTiles(grid)
    -- Wandelt 2D-Grid in flaches Tile-Array für JSON um.
    local tiles = {}
    local height = #grid
    local width = 0
    if height > 0 and type(grid[1]) == "table" then
        width = #grid[1]
    end

    for y = 1, height do
        local row = grid[y]
        for x = 1, width do
            tiles[#tiles + 1] = tonumber(row[x]) or 0
        end
    end

    return width, height, tiles
end

local function exportRoom(levelPath, outPath, levelName)
    -- Exportiert eine Raum-Lua-Datei als Raum-JSON.
    local data, err = readLuaTable(levelPath)
    if not data then return false, "Load failed: " .. tostring(err) end

    local grid = data.grid or data
    if type(grid) ~= "table" then
        return false, "No valid grid in " .. levelName
    end

    local width, height, tiles = flattenTiles(grid)
    local payload = {
        name = levelName,
        width = width,
        height = height,
        tiles = tiles,
    }

    -- Items aus Editor-Daten übernehmen
    if data.items and type(data.items) == "table" then
        print("DEBUG: map " .. levelName .. " has item count " .. #data.items)
    end
    if data.items and type(data.items) == "table" and #data.items > 0 then
        payload.items = {}
        for _, it in ipairs(data.items) do
            table.insert(payload.items, { x = it.x, y = it.y, type = it.type })
        end
    end

    -- Zusatz: bestimmte Spezial-Tiles automatisch in Items übersetzen.
    -- Colors/IDs orientieren sich an Constants.PLACEHOLDER_TILES.
    -- zugrunde liegende tiles[] werden auf 0 gesetzt (Luft).
    local placeholderId = Constants.PLACEHOLDER_TILES and Constants.PLACEHOLDER_TILES.double_jump
    if placeholderId then
        for y = 1, height do
            for x = 1, width do
                local idx = (y-1) * width + x
                if payload.tiles[idx] == placeholderId then
                    -- Item record hinzufügen
                    payload.items = payload.items or {}
                    table.insert(payload.items, { x = x, y = y, type = "double_jump" })
                    payload.tiles[idx] = 0
                end
            end
        end
    end

    local ok, writeErr = writeText(outPath, jsonEncode(payload))
    if not ok then return false, "Write failed: " .. tostring(writeErr) end
    return true
end

local function exportWorld(worldPath, outPath)
    -- Exportiert world.lua als world.json mit Cells + Checkpoints.
    local data, err = readLuaTable(worldPath)
    if not data then return false, "Load failed: " .. tostring(err) end

    local grid = data.grid or {}
    local checkpoints = data.checkpoints or {}

    local cells = {}
    for key, levelName in pairs(grid) do
        local xs, ys = tostring(key):match("^(-?%d+),(-?%d+)$")
        if xs and ys then
            cells[#cells + 1] = {
                x = tonumber(xs),
                y = tonumber(ys),
                level = tostring(levelName),
            }
        end
    end

    table.sort(cells, function(a, b)
        if a.y == b.y then return a.x < b.x end
        return a.y < b.y
    end)

    local cpOut = {}
    for key, value in pairs(checkpoints) do
        if value then cpOut[tostring(key)] = true end
    end

    local payload = {
        cells = cells,
        checkpoints = cpOut,
    }

    local ok, writeErr = writeText(outPath, jsonEncode(payload))
    if not ok then return false, "Write failed: " .. tostring(writeErr) end
    return true
end

function MapExporter.exportAll()
    -- Hauptfunktion: exportiert alle .lua-Leveldateien in romfs/maps.
    local editorRoot = love.filesystem.getSource():gsub("/", "\\")
    local levelDir = editorRoot .. "\\level"
    local mapsDir = editorRoot .. "\\..\\3ds-cpp\\romfs\\maps"

    local mk = os.execute('if not exist "' .. mapsDir .. '" mkdir "' .. mapsDir .. '"')
    if mk == false then
        return false, "Could not create maps directory: " .. mapsDir
    end

    local exported = 0
    local cmd = 'powershell -command "Get-ChildItem -Path \"' .. levelDir .. '\" -Filter *.lua | Select-Object -ExpandProperty Name"'
    local handle = io.popen(cmd)
    if not handle then return false, "Failed to list level files" end
    local output = handle:read("*a") or ""
    handle:close()

    for name in output:gmatch("[^\r\n]+") do
        if name ~= "" then
            local levelName = name:gsub("%.lua$", "")
            local inPath = levelDir .. "\\" .. name
            if levelName == "world" then
                local outPath = mapsDir .. "\\world.json"
                local ok, err = exportWorld(inPath, outPath)
                if not ok then return false, err end
                exported = exported + 1
            else
                local outPath = mapsDir .. "\\" .. levelName .. ".json"
                local ok, err = exportRoom(inPath, outPath, levelName)
                if not ok then return false, err end
                exported = exported + 1
            end
        end
    end

    return true, string.format("Export fertig. %d Datei(en) geschrieben nach %s", exported, mapsDir)
end

return MapExporter
