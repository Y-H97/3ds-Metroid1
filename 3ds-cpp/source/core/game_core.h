#pragma once

#include <cstdint>
#include <vector>

struct InputState {
    bool left = false;
    bool right = false;
    bool jump = false;
    bool jumpPressed = false;
};

struct Player {
    float x = 40.0f;
    float y = 40.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float w = 20.0f;
    float h = 30.0f;
    bool grounded = false;
    float coyoteTimer = 0.0f;
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

class TileMap {
public:
    int width = 0;
    int height = 0;
    std::vector<uint8_t> tiles;

    bool loadText(const char* path);
    bool loadJson(const char* path);
    bool isSolid(int tx, int ty) const;
    int getTile(int tx, int ty) const;
    const std::vector<Rect>& getTransitions() const { return transitions; }

    void buildTransitions(int tileSize);

private:
    std::vector<Rect> transitions;
};

class GameCore {
public:
    bool loadMapJson(const char* path);
    bool loadMapText(const char* path);

    void update(const InputState& input, float dt);

    const Player& getPlayer() const { return player; }
    Player& getPlayer() { return player; }

    const TileMap& getMap() const { return map; }
    TileMap& getMap() { return map; }

    void setPlayerStart(float x, float y);
    bool setPlayerStartToFirstEmpty(float tileSize);
    void setPlayerPosition(float x, float y);
    bool consumeDeath();

private:
    TileMap map;
    Player player;
    float spawnX = 40.0f;
    float spawnY = 40.0f;
    bool dead = false;
};
