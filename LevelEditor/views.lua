local menu = require "views.menu"
local room = require "views.room_editor"
local world = require "views.world_editor"

-- Dieses Modul sammelt die Zeichenfunktionen der einzelnen Editor-Ansichten
-- und stellt sie als gemeinsame Tabelle fuer die Hauptlogik bereit.
local Views = {}
Views.drawMenu = menu.drawMenu
Views.drawNewMapMenu = menu.drawNewMapMenu
Views.drawRoomEditor = room.drawRoomEditor
Views.drawWorldEditor = world.drawWorldEditor

return Views
