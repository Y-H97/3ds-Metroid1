#include "game_core.h"

#include <cstdio>
#include <string>

namespace {

int findJsonInt(const std::string& json, const char* key) {
    std::string token = std::string("\"") + key + "\":";
    size_t pos = json.find(token);
    if (pos == std::string::npos) {
        return -1;
    }

    pos += token.size();
    int value = -1;
    std::sscanf(json.c_str() + pos, "%d", &value);
    return value;
}

void parseJsonItems(const std::string& json, std::vector<TileMap::ItemData>& outItems) {
    outItems.clear();
    size_t itemsPos = json.find("\"items\"");
    if (itemsPos == std::string::npos) {
        return;
    }

    size_t arrStart = json.find('[', itemsPos);
    if (arrStart == std::string::npos) {
        return;
    }

    size_t index = arrStart + 1;
    while (index < json.size()) {
        while (index < json.size() && json[index] != '{' && json[index] != ']') {
            ++index;
        }
        if (index >= json.size() || json[index] == ']') {
            break;
        }

        size_t objStart = index;
        int braceDepth = 1;
        ++index;
        while (index < json.size() && braceDepth > 0) {
            if (json[index] == '{') braceDepth++;
            else if (json[index] == '}') braceDepth--;
            ++index;
        }

        std::string object = json.substr(objStart, index - objStart);
        std::string type;
        int itemX = 0;
        int itemY = 0;

        size_t typePos = object.find("\"type\"");
        if (typePos != std::string::npos) {
            size_t colon = object.find(':', typePos);
            size_t quote1 = colon == std::string::npos ? std::string::npos : object.find('"', colon + 1);
            size_t quote2 = quote1 == std::string::npos ? std::string::npos : object.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                type = object.substr(quote1 + 1, quote2 - quote1 - 1);
            }
        }

        size_t posX = object.find("\"x\"");
        if (posX != std::string::npos) {
            std::sscanf(object.c_str() + posX, "\"x\"%*[^0-9-]%d", &itemX);
        }

        size_t posY = object.find("\"y\"");
        if (posY != std::string::npos) {
            std::sscanf(object.c_str() + posY, "\"y\"%*[^0-9-]%d", &itemY);
        }

        if (!type.empty()) {
            TileMap::ItemData item;
            item.type = type;
            item.x = itemX;
            item.y = itemY;
            outItems.push_back(item);
        }
    }
}

void convertPlaceholderTilesToItems(TileMap& map) {
    // Editor-Platzhalter werden beim Laden in echte Item-Eintraege umgewandelt.
    const uint8_t placeholderDoubleJump = 99;
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            size_t tileIndex = static_cast<size_t>(y * map.width + x);
            if (map.tiles[tileIndex] != placeholderDoubleJump) {
                continue;
            }

            bool exists = false;
            for (const auto& item : map.items) {
                if (item.x == x && item.y == y) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                TileMap::ItemData item;
                item.type = "double_jump";
                item.x = x;
                item.y = y;
                map.items.push_back(item);
            }
            map.tiles[tileIndex] = 0;
        }
    }
}

} // namespace

bool TileMap::loadText(const char* path) {
    // Legacy-Textformat laden: zuerst width/height, dann alle Tile-Werte.
    FILE* file = fopen(path, "r");
    if (!file) return false;

    if (fscanf(file, "%d %d", &width, &height) != 2) {
        fclose(file);
        return false;
    }

    tiles.resize(static_cast<size_t>(width * height));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int value = 0;
            if (fscanf(file, "%d", &value) != 1) {
                fclose(file);
                return false;
            }
            tiles[static_cast<size_t>(y * width + x)] = static_cast<uint8_t>(value);
        }
    }

    fclose(file);
    buildTransitions(16);
    return true;
}

bool TileMap::loadJson(const char* path) {
    // Sehr einfacher JSON-Reader (ohne externe Bibliothek), ausreichend fuer das Kartenformat.
    FILE* file = fopen(path, "r");
    if (!file) return false;

    std::string json;
    char buffer[512];
    while (size_t count = fread(buffer, 1, sizeof(buffer), file)) {
        json.append(buffer, count);
    }
    fclose(file);

    int mapWidth = findJsonInt(json, "width");
    int mapHeight = findJsonInt(json, "height");
    if (mapWidth <= 0 || mapHeight <= 0) {
        return false;
    }

    width = mapWidth;
    height = mapHeight;
    tiles.assign(static_cast<size_t>(mapWidth * mapHeight), 0);

    size_t tilesPos = json.find("\"tiles\"");
    if (tilesPos == std::string::npos) return false;
    tilesPos = json.find('[', tilesPos);
    if (tilesPos == std::string::npos) return false;

    size_t index = tilesPos + 1;
    size_t tileIndex = 0;
    while (index < json.size() && tileIndex < tiles.size()) {
        while (index < json.size() && !(json[index] == '-' || (json[index] >= '0' && json[index] <= '9'))) {
            ++index;
        }
        if (index >= json.size() || json[index] == ']') {
            break;
        }

        int value = 0;
        int consumed = 0;
        if (std::sscanf(json.c_str() + index, "%d%n", &value, &consumed) != 1) {
            break;
        }
        tiles[tileIndex++] = static_cast<uint8_t>(value);
        index += consumed;
    }

    bool ok = tileIndex == tiles.size();
    if (!ok) {
        return false;
    }

    buildTransitions(16);
    parseJsonItems(json, items);
    convertPlaceholderTilesToItems(*this);
    return true;
}
