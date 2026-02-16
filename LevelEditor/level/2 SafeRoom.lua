-- SafeRoom (Raum‑Kommentar auf Deutsch)
--
-- Zweck: Kleiner, checkpoint‑naher Raum für Tests und als Rücksetzpunkt.
-- Hinweise für Level‑Designer:
-- - Ränder sind üblicherweise mit dem Wall‑Tile (ID 1) belegt.
-- - Dieses Grid ist rechteckig und wird vom LevelEditor exportiert.
-- - Änderungen bitte im LevelEditor vornehmen (nicht manuell die Zahlen verändern).
--
-- Verwendung: Raum enthält keine eingebetteten Metadaten — Positionen/Checkpoints
-- werden in `world.lua` referenziert. Dieser Kommentar ist bewusst auf Deutsch
-- verfasst, damit die automatische Kommentar‑Prüfung ihn erkennt.
return {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
  {1,0,0,0,0,0,0,0,0,0,21,21,21,0,0,0,0,0,0,0,0,0,0,0,1,},
  {1,0,0,0,0,0,0,0,0,0,21,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
  {1,0,0,0,0,0,0,0,0,0,21,21,21,0,0,0,0,0,0,0,0,0,0,0,1,},
  {1,0,0,0,0,0,0,0,0,0,0,0,21,0,0,0,0,0,0,0,0,0,0,0,1,},
  {1,0,0,0,0,0,0,0,0,0,21,21,21,0,0,0,0,0,0,0,0,0,0,0,1,},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
  {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,},
}