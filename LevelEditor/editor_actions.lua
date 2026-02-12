local State = require "state"
local Constants = require "constants"
local IOUtils = require "io_utils"

local Actions = {}

-- Erstellt eine neue, leere Map mit den angegebenen Maßen (in Screens)
function Actions.createMap(w, h)
    State.mapScreensW = w
    State.mapScreensH = h
    State.currentRoom = {}
    State.currentObjects = {}
    
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
    content = content .. "  },\n"
    
    -- Objekte speichern
    content = content .. "  objects = {\n"
    for _, obj in ipairs(State.currentObjects) do
        local shapeStr = 'nil'
        if obj.shape then shapeStr = '"' .. obj.shape .. '"' end
        -- Formatierung mit 2 Nachkommastellen für Sauberkeit
        content = content .. "    {x="..string.format("%.2f", obj.x)..", y="..string.format("%.2f", obj.y)..", w="..string.format("%.2f", obj.w)..", h="..string.format("%.2f", obj.h)..", type="..obj.type..", shape="..shapeStr.."},\n"
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

-- Hilfsfunktion für Nachrichten
function Actions.setMessage(msg, time)
    State.message = msg
    State.messageTimer = time
end

return Actions
