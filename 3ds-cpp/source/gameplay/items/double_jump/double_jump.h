#pragma once

#include <string>

// Dieses Modul kapselt das Verhalten des Doppelsprung-Upgrades.
// Es lebt in einem eigenen Verzeichnis, damit weitere Items analog
// hinzugefügt und unabhängig bearbeitet werden können.

namespace items {
namespace double_jump {

// Liefert die interne ID (Typstring) des Items.
inline const std::string& id() {
    static const std::string s = "double_jump";
    return s;
}

// Wird aufgerufen, sobald der Spieler das Item eingesammelt hat.
// Die mitgelieferte Szene kann benutzt werden, um dem Spieler die
// doppelte Sprungfähigkeit zu geben und eine Nachricht anzuzeigen.
void onCollect(class GameplayScene& scene);

} // namespace double_jump
} // namespace items
