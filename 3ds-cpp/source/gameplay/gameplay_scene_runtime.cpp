#include "gameplay_scene.h"

void GameplayScene::updateFpsCounter() {
    // Laufende FPS-Messung fuer das optionale Debug-Overlay.
    fpsFrameCounter++;
    u64 nowMs = osGetTime();
    if (nowMs - fpsTimerMs >= 1000) {
        fpsValue = fpsFrameCounter;
        fpsFrameCounter = 0;
        fpsTimerMs = nowMs;
    }
}

void GameplayScene::update(float dt) {
    // Koordiniert nur noch die groben Runtime-Phasen eines einzelnen Frames.
    InputState input;
    input.left = moveLeftHeld;
    input.right = moveRightHeld;
    input.jump = jumpHeld;
    input.jumpPressed = jumpPressed;

    core.update(input, dt);
    jumpPressed = false;

    updatePlayerWorldState();
    collectOverlappingItems();
    handleMapTransition();
    respawnFromCheckpoint();
    updateFpsCounter();
    updatePickupMessageTimer(dt);
}
