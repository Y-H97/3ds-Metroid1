local State = require "state"
local UI = require "ui_components"
local Actions = require "editor_actions"

local M = {}

function M.drawWorldEditor()
    local sw, sh = love.graphics.getDimensions()
    -- Toolbar
    if UI.drawButton("💾", 10, 10, 34, 28) then
        Actions.saveWorld()
    end
    if UI.drawButton("📂", 52, 10, 34, 28) then
        Actions.loadWorld()
    end
    if UI.drawButton("🔄", 94, 10, 34, 28) then
        Actions.refreshWorldFileList()
        Actions.setMessage("Liste aktualisiert!", 1.5)
    end
    if UI.drawButton("?", 136, 10, 28, 28) then
        State.showHelp = not State.showHelp
    end

    love.graphics.setColor(1, 1, 1)
    love.graphics.printf("[S] Speichern | [L] Laden | [R] Refresh | [C] Checkpoint | [ESC] Menü | [F11] Vollbild | [H] Hilfe", 180, 44, sw - 200)

    if State.message ~= "" then
        love.graphics.setColor(0, 1, 0)
        love.graphics.print(UI.sanitizeForDisplay(State.message), 10, 30)
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

            -- Draw room box
            local meta = State.roomCache[roomName] or {w=1, h=1}
            love.graphics.setColor(0.2, 0.5, 0.2, 0.2)
            love.graphics.rectangle("fill", rx, ry, meta.w * cellSize, meta.h * cellSize)
            love.graphics.setColor(0.9, 0.9, 0.9)
            love.graphics.rectangle("line", rx, ry, meta.w * cellSize, meta.h * cellSize)
            love.graphics.print(roomName, rx + 6, ry + 6)

            -- Checkpoint Marker
            if State.worldCheckpoints[key] then
                love.graphics.setColor(1, 0.8, 0.2)
                love.graphics.circle("fill", rx + 12, ry + 12, 6)
            end
        end
    end

    love.graphics.pop()
    love.graphics.setScissor()
end

return M
