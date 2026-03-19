#pragma once

#include <string>

// Forward declaration
class GameplayScene;

// Namespace für Item-Definitionen und -Verhalten
namespace items {

    // Doppelsprung-Item
    namespace double_jump {
        /// Gibt die eindeutige ID dieses Items zurück
        inline const char* id() {
            return "double_jump";
        }

        /// Wird aufgerufen, wenn der Spieler das Item einsammelt
        inline void onCollect(GameplayScene& scene) {
            // Diese Funktion wird aufgerufen, wenn der Spieler das Doppelsprung-Item einsammelt.
            // Die Flag wird bereits von GameplayScene::update() gesetzt bevor onCollect() aufgerufen wird,
            // daher ist hier eigentlich nichts zu tun. Man könnte hier aber zusätzliche Effekte
            // hinzufügen wie Sound, Partikel, Message anzeigen, etc.
            
            (void)scene; // Vermeidung von Compiler-Warning bei Variabler die nicht genutzt wird
        }
    }

}
