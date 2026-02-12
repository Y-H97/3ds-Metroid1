local UI = {}

-- Zeichnet einen simplen Button und gibt true zurück, wenn er geklickt wurde
function UI.drawButton(text, x, y, w, h, callback)
    local mouseX, mouseY = love.mouse.getPosition()
    local hover = mouseX >= x and mouseX <= x + w and mouseY >= y and mouseY <= y + h
    
    -- Hover-Effekt
    if hover then
        love.graphics.setColor(0.4, 0.4, 0.4)
        if love.mouse.isDown(1) then
            love.graphics.setColor(0.3, 0.3, 0.3) -- Klick Farbe
        end
    else
        love.graphics.setColor(0.2, 0.2, 0.2) -- Standard Farbe
    end
    
    -- Hintergrund
    love.graphics.rectangle("fill", x, y, w, h)
    
    -- Rahmen
    love.graphics.setColor(1, 1, 1)
    love.graphics.rectangle("line", x, y, w, h)
    
    -- Text zentrieren
    local font = love.graphics.getFont()
    local tw = font:getWidth(text)
    local th = font:getHeight()
    love.graphics.print(text, x + (w-tw)/2, y + (h-th)/2)
    
    -- Logik: Wurde geklickt?
    -- Einfache Implementierung: Gibt true zurück solange Taste gedrückt ist.
    -- Eine "OnRelease" Logik wäre sauberer, aber für Editor-Tools oft okay.
    return hover and love.mouse.isDown(1) 
end

-- Bereinigt Strings für die Anzeige (ersetzt nicht-druckbare Zeichen)
-- Wichtig da Lua Strings binär sein können.
function UI.sanitizeForDisplay(str)
    local newStr = ""
    for i = 1, #str do
        local b = string.byte(str, i)
        if b < 128 then
            newStr = newStr .. string.char(b)
        else
            newStr = newStr .. "?" 
        end
    end
    return newStr
end

return UI
