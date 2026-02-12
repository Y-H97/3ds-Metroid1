local IOUtils = {}

-- Gibt den absoluten Pfad zum Source-Verzeichnis zurück
-- Wichtig für PowerShell-Befehle, da LÖVEs Dateisystem isoliert ist.
function IOUtils.getProjectDir()
    return love.filesystem.getSource():gsub("/", "\\")
end

-- Hilfsfunktion: String zu Base64 kodieren (für sichere Übergabe an PowerShell)
function IOUtils.toBase64(str)
    return love.data.encode("string", "base64", str)
end

-- Hilfsfunktion: Base64 zu String dekodieren
function IOUtils.fromBase64(str)
    return love.data.decode("string", "base64", str)
end

-- Zeigt einen Windows "Datei Öffnen" Dialog via PowerShell
function IOUtils.showOpenDialog()
    local dir = IOUtils.getProjectDir() .. "\\level"
    local dirB64 = IOUtils.toBase64(dir)
    
    -- PowerShell command to open FileDialog
    -- Wir nutzen Base64 für Pfade, um Probleme mit Leerzeichen/Sonderzeichen zu vermeiden.
    local cmd = 'powershell -command "$path = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String(\'' .. dirB64 .. '\')); Add-Type -AssemblyName System.Windows.Forms; $f = New-Object System.Windows.Forms.OpenFileDialog; $f.InitialDirectory = $path; $f.Filter = \'Lua Files (*.lua)|*.lua\'; if ($f.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) { [System.Convert]::ToBase64String([System.Text.Encoding]::UTF8.GetBytes($f.FileName)) }"'
    
    local handle = io.popen(cmd)
    local result = handle:read("*a")
    handle:close()
    
    if result then 
        result = result:gsub("\n", ""):gsub("\r", "") 
        if result == "" then return nil end
        return IOUtils.fromBase64(result)
    end
    return nil
end

-- Zeigt einen Windows "Datei Speichern" Dialog via PowerShell
function IOUtils.showSaveDialog()
    local dir = IOUtils.getProjectDir() .. "\\level"
    local dirB64 = IOUtils.toBase64(dir)
    
    local cmd = 'powershell -command "$path = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String(\'' .. dirB64 .. '\')); Add-Type -AssemblyName System.Windows.Forms; $f = New-Object System.Windows.Forms.SaveFileDialog; $f.InitialDirectory = $path; $f.Filter = \'Lua Files (*.lua)|*.lua\'; if ($f.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) { [System.Convert]::ToBase64String([System.Text.Encoding]::UTF8.GetBytes($f.FileName)) }"'
    
    local handle = io.popen(cmd)
    local result = handle:read("*a")
    handle:close()
    
    if result then 
        result = result:gsub("\n", ""):gsub("\r", "") 
        if result == "" then return nil end
        return IOUtils.fromBase64(result)
    end
    return nil
end

-- Liest eine Datei direkt vom Dateisystem (umgeht LÖVE Sandbox)
-- Fallback auf PowerShell, wenn io.open fehlschlägt (z.B. Permissions)
function IOUtils.readFilePS(path)
    if not path then return nil end
    
    -- 1. Versuch: Standard Lua IO
    local f = io.open(path, "rb")
    if f then
        local content = f:read("*a")
        f:close()
        return content
    end
    
    print("IOUtils: io.open fehlgeschlagen. Nutze PowerShell Fallback.")
    
    -- 2. Fallback: PowerShell
    local pathB64 = IOUtils.toBase64(path)
    local cmd = 'powershell -command "$path = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String(\'' .. pathB64 .. '\')); [System.Convert]::ToBase64String([System.IO.File]::ReadAllBytes($path))"'
    
    local handle = io.popen(cmd)
    local result = handle:read("*a")
    handle:close()
    
    if result then 
        result = result:gsub("\n", ""):gsub("\r", "") 
        if result == "" then return nil end
        return IOUtils.fromBase64(result)
    end
    return nil
end

-- Schreibt eine Datei direkt auf das Dateisystem
-- Nutzt Chunked-Writing via PowerShell für große Dateien, um CMD-Limits zu umgehen.
function IOUtils.writeFilePS(path, content)
    -- 1. Versuch: Lua Standard IO
    local f, err = io.open(path, "wb")
    if f then
        f:write(content)
        f:close()
        -- print("Datei geschrieben (IO): " .. path)
        return true
    else
        print("Fehler IO: " .. tostring(err))
    end
    
    -- 2. Fallback: PowerShell (Chunked)
    -- print("Nutze PowerShell Fallback (Chunked)...")
    
    local pathB64 = IOUtils.toBase64(path)
    
    -- Schritt A: Datei leeren/erstellen
    local createCmd = 'powershell -command "$path = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String(\'' .. pathB64 .. '\')); [System.IO.File]::WriteAllText($path, \'\')"'
    local h = io.popen(createCmd)
    h:close()
    
    -- Schritt B: Content stückweise anhängen (um 8KB Command Line Limit zu vermeiden)
    local chunkSize = 2000 
    local len = #content
    
    for i = 1, len, chunkSize do
        local chunk = content:sub(i, math.min(i + chunkSize - 1, len))
        local chunkB64 = IOUtils.toBase64(chunk)
        
        local appendCmd = 'powershell -command "$path = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String(\'' .. pathB64 .. '\')); $bytes = [System.Convert]::FromBase64String(\'' .. chunkB64 .. '\'); [System.IO.File]::AppendAllText($path, [System.Text.Encoding]::UTF8.GetString($bytes))"'
        
        local hChunk = io.popen(appendCmd)
        hChunk:close()
    end
    -- print("Datei geschrieben (PS Chunked): " .. path)
end

return IOUtils
