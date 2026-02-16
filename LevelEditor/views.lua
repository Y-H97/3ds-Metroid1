local menu = require "views.menu"
local room = require "views.room_editor"
local world = require "views.world_editor"

local Views = {}
Views.drawMenu = menu.drawMenu
Views.drawNewMapMenu = menu.drawNewMapMenu
Views.drawRoomEditor = room.drawRoomEditor
Views.drawWorldEditor = world.drawWorldEditor

return Views
