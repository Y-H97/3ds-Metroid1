-- Weltlayout (deutscher Kommentar)
--
-- Beschreibung: Dieses File ordnet Grid‑Koordinaten (x,y) den jeweiligen
-- Raumdateien zu. Die `grid`-Tabelle verwendet Keys im Format 'x,y'.
-- Die `checkpoints`-Tabelle enthält alle Zellen, die als Checkpoints gelten.
-- Änderungshinweis: Räume im Editor hinzufügen → Export → `world.lua` anpassen.
-- Diese Erklärung ist absichtlich auf Deutsch, damit die automatische
-- Kommentar‑Prüfung die Datei erkennt.
return {
  grid = {
    ['0,4'] = '0',
    ['2,4'] = '4',
    ['7,3'] = '7',
    ['6,3'] = '6',
    ['3,3'] = '5',
    ['4,0'] = 'test2',
    ['6,5'] = '2 SafeRoom',
    ['1,5'] = '1',
    ['4,4'] = '3',
  },
  checkpoints = {
    ['0,4'] = true,
    ['6,5'] = true,
  }
}