#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "../source/core/game_core.h"

namespace {

struct SimulatorState {
    GameCore core;
    std::string mapPath;
    float dt = 1.0f / 60.0f;
    bool holdLeft = false;
    bool holdRight = false;
    bool holdJump = false;
    float spawnX = 40.0f;
    float spawnY = 40.0f;
};

std::string trim(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(start, end - start);
}

char tileToChar(int tile) {
    switch (tile) {
        case 1:
            return '#';
        case 2:
            return '^';
        case 3:
            return '>';
        case 30:
            return '/';
        case 31:
            return '\\';
        case 32:
            return 'V';
        case 33:
            return 'A';
        default:
            return '.';
    }
}

bool loadMap(SimulatorState& state, const std::string& path) {
    if (!state.core.loadMapJson(path.c_str())) {
        std::cout << "Konnte Karte nicht laden: " << path << "\n";
        return false;
    }

    if (!state.core.setPlayerStartToFirstEmpty(16.0f)) {
        state.core.setPlayerStart(40.0f, 40.0f);
    }

    const Player& p = state.core.getPlayer();
    state.spawnX = p.x;
    state.spawnY = p.y;
    state.mapPath = path;
    std::cout << "Map geladen: " << path << "\n";
    return true;
}

void printStatus(const SimulatorState& state) {
    const Player& p = state.core.getPlayer();
    std::cout << "Pos(" << p.x << ", " << p.y << ") "
              << "Vel(" << p.vx << ", " << p.vy << ") "
              << "Grounded=" << (p.grounded ? "ja" : "nein") << " "
              << "Hold[L=" << (state.holdLeft ? "1" : "0")
              << " R=" << (state.holdRight ? "1" : "0")
              << " J=" << (state.holdJump ? "1" : "0") << "]\n";
}

void printViewport(const SimulatorState& state, int viewW = 40, int viewH = 20) {
    const TileMap& map = state.core.getMap();
    const Player& p = state.core.getPlayer();

    if (map.width <= 0 || map.height <= 0) {
        std::cout << "Keine Map geladen.\n";
        return;
    }

    int playerTx = static_cast<int>((p.x + p.w * 0.5f) / 16.0f);
    int playerTy = static_cast<int>((p.y + p.h * 0.5f) / 16.0f);

    int startX = playerTx - viewW / 2;
    int startY = playerTy - viewH / 2;

    std::cout << "\n--- Viewport ---\n";
    for (int y = 0; y < viewH; ++y) {
        int ty = startY + y;
        for (int x = 0; x < viewW; ++x) {
            int tx = startX + x;
            if (tx == playerTx && ty == playerTy) {
                std::cout << '@';
                continue;
            }

            int t = map.getTile(tx, ty);
            if (t < 0) {
                std::cout << ' ';
            } else {
                std::cout << tileToChar(t);
            }
        }
        std::cout << "\n";
    }
    std::cout << "----------------\n";
}

void runFrames(SimulatorState& state,
               int frames,
               bool stepLeft,
               bool stepRight,
               bool stepJump) {
    if (frames < 1) frames = 1;

    for (int i = 0; i < frames; ++i) {
        InputState input;
        input.left = state.holdLeft || stepLeft;
        input.right = state.holdRight || stepRight;
        input.jump = state.holdJump || stepJump;
        input.jumpPressed = (i == 0) && (state.holdJump || stepJump);

        state.core.update(input, state.dt);
        if (state.core.consumeDeath()) {
            std::cout << "Tod erkannt -> Respawn auf Spawnpunkt.\n";
            state.core.setPlayerPosition(state.spawnX, state.spawnY);
        }
    }
}

void printHelp() {
    std::cout
        << "\nBefehle:\n"
        << "  help                     Hilfe anzeigen\n"
        << "  status                   Spielerstatus anzeigen\n"
        << "  view [w h]               ASCII-Viewport anzeigen\n"
        << "  step [n] [keys]          n Frames simulieren, keys: l r j\n"
        << "  hold <l|r|j> <on|off>    Dauerinput setzen\n"
        << "  tp <x> <y>               Spielerposition setzen\n"
        << "  reset                    Auf Spawnpunkt zurücksetzen\n"
        << "  map <pfad>               Neue JSON-Map laden\n"
        << "  quit                     Simulator beenden\n";
}

}  // namespace

int main(int argc, char** argv) {
    SimulatorState state;
    std::string initialMap = "romfs/maps/test.json";
    if (argc >= 2) {
        initialMap = argv[1];
    }

    std::cout << "Metroid Console Simulator\n";
    std::cout << "Standard-Map: " << initialMap << "\n";

    if (!loadMap(state, initialMap)) {
        std::cout << "Tipp: map romfs/maps/0.json\n";
    }

    printHelp();
    printStatus(state);
    printViewport(state);

    std::string line;
    while (true) {
        std::cout << "\nsim> ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        line = trim(line);
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "quit" || cmd == "exit" || cmd == "q") {
            break;
        }
        if (cmd == "help") {
            printHelp();
            continue;
        }
        if (cmd == "status") {
            printStatus(state);
            continue;
        }
        if (cmd == "view") {
            int w = 40;
            int h = 20;
            iss >> w >> h;
            if (w < 8) w = 8;
            if (h < 6) h = 6;
            printViewport(state, w, h);
            continue;
        }
        if (cmd == "step") {
            int frames = 1;
            std::string keys;
            iss >> frames >> keys;
            bool left = keys.find('l') != std::string::npos;
            bool right = keys.find('r') != std::string::npos;
            bool jump = keys.find('j') != std::string::npos;
            runFrames(state, frames, left, right, jump);
            printStatus(state);
            printViewport(state);
            continue;
        }
        if (cmd == "hold") {
            std::string key;
            std::string value;
            iss >> key >> value;
            bool on = (value == "on" || value == "1" || value == "true");
            if (key == "l") {
                state.holdLeft = on;
            } else if (key == "r") {
                state.holdRight = on;
            } else if (key == "j") {
                state.holdJump = on;
            } else {
                std::cout << "Unbekannter Key: " << key << " (erlaubt: l/r/j)\n";
                continue;
            }
            printStatus(state);
            continue;
        }
        if (cmd == "tp") {
            float x = 0.0f;
            float y = 0.0f;
            if (!(iss >> x >> y)) {
                std::cout << "Verwendung: tp <x> <y>\n";
                continue;
            }
            state.core.setPlayerPosition(x, y);
            printStatus(state);
            printViewport(state);
            continue;
        }
        if (cmd == "reset") {
            state.core.setPlayerPosition(state.spawnX, state.spawnY);
            printStatus(state);
            printViewport(state);
            continue;
        }
        if (cmd == "map") {
            size_t firstSpace = line.find(' ');
            if (firstSpace == std::string::npos) {
                std::cout << "Verwendung: map <pfad>\n";
                continue;
            }
            std::string path = trim(line.substr(firstSpace + 1));
            if (!loadMap(state, path)) {
                continue;
            }
            printStatus(state);
            printViewport(state);
            continue;
        }

        std::cout << "Unbekannter Befehl: " << cmd << " (help für Liste)\n";
    }

    std::cout << "Simulator beendet.\n";
    return 0;
}
