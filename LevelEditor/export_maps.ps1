param(
    [string]$LevelDir = "$PSScriptRoot\level",
    [string]$MapsDir = "$PSScriptRoot\..\3ds-cpp\romfs\maps"
)

$ErrorActionPreference = 'Stop'

# Konvertiert eine einzelne Raum-Lua-Datei in das JSON-Raumformat.
function Convert-RoomLuaToJson {
    param(
        [string]$Path,
        [string]$OutPath
    )

    $name = [System.IO.Path]::GetFileNameWithoutExtension($Path)
    $content = Get-Content -Path $Path -Raw -Encoding UTF8
    $lines = $content -split "`r?`n"

    $inGrid = $false
    $hasExplicitGrid = $false
    $inTopLevel = $false
    $rows = @()

    foreach ($line in $lines) {
        if (-not $inGrid -and $line -match '^\s*grid\s*=\s*\{') {
            $inGrid = $true
            $hasExplicitGrid = $true
            continue
        }

        if (-not $hasExplicitGrid -and -not $inTopLevel -and $line -match '^\s*return\s*\{') {
            $inTopLevel = $true
            continue
        }

        if ($inGrid -and $line -match '^\s*\},\s*$') {
            break
        }

        if ($inTopLevel -and $line -match '^\s*\}\s*$') {
            break
        }

        if (($inGrid -or $inTopLevel) -and $line -match '^\s*\{') {
            if ($line -match '=') { continue }
            $numMatches = [regex]::Matches($line, '-?\d+')
            if ($numMatches.Count -gt 0) {
                $row = @()
                foreach ($m in $numMatches) {
                    $row += [int]$m.Value
                }
                $rows += ,$row
            }
        }
    }

    if ($rows.Count -eq 0) {
        throw "Kein Grid in $Path gefunden"
    }

    $width = $rows[0].Count
    $height = $rows.Count
    $tiles = New-Object System.Collections.Generic.List[int]

    foreach ($row in $rows) {
        if ($row.Count -ne $width) {
            throw "Uneinheitliche Zeilenbreite in $Path (erwartet $width, gefunden $($row.Count))"
        }
        foreach ($value in $row) {
            [void]$tiles.Add([int]$value)
        }
    }

    $obj = [ordered]@{
        name = $name
        width = $width
        height = $height
        tiles = $tiles
    }

    $json = $obj | ConvertTo-Json -Depth 4 -Compress
    Set-Content -Path $OutPath -Value $json -Encoding UTF8
}

function Convert-WorldLuaToJson {
    # Konvertiert world.lua in world.json (cells + checkpoints).
    param(
        [string]$Path,
        [string]$OutPath
    )

    $content = Get-Content -Path $Path -Raw -Encoding UTF8

    $cellMatches = [regex]::Matches($content, "\['(-?\d+),(-?\d+)'\]\s*=\s*'([^']+)'")
    $cells = @()
    foreach ($m in $cellMatches) {
        $cells += [ordered]@{
            x = [int]$m.Groups[1].Value
            y = [int]$m.Groups[2].Value
            level = $m.Groups[3].Value
        }
    }

    $cells = $cells | Sort-Object y, x

    $cpMatches = [regex]::Matches($content, "\['(-?\d+,-?\d+)'\]\s*=\s*true")
    $checkpoints = [ordered]@{}
    foreach ($m in $cpMatches) {
        $checkpoints[$m.Groups[1].Value] = $true
    }

    $obj = [ordered]@{
        cells = $cells
        checkpoints = $checkpoints
    }

    $json = $obj | ConvertTo-Json -Depth 6 -Compress
    Set-Content -Path $OutPath -Value $json -Encoding UTF8
}

function Validate-RoomJson {
    # Prüft, ob width*height zur Anzahl der Tiles passt.
    param(
        [string]$Path
    )

    $room = Get-Content -Path $Path -Raw -Encoding UTF8 | ConvertFrom-Json
    $expected = [int]$room.width * [int]$room.height
    $actual = @($room.tiles).Count
    if ($actual -ne $expected) {
        throw "Ungültige Raumdaten in ${Path}: tiles=$actual, erwartet=$expected"
    }
}

function Validate-WorldJson {
    # Prüft, dass world.json nur vorhandene Räume referenziert.
    param(
        [string]$Path,
        [hashtable]$AvailableRooms
    )

    $world = Get-Content -Path $Path -Raw -Encoding UTF8 | ConvertFrom-Json
    foreach ($cell in @($world.cells)) {
        if (-not $AvailableRooms.ContainsKey([string]$cell.level)) {
            throw "Welt referenziert unbekannten Raum '$($cell.level)' in ${Path}"
        }
    }
}

if (-not (Test-Path $LevelDir)) {
    # Export-Voraussetzungen prüfen und Zielordner anlegen.
    throw "Level-Verzeichnis nicht gefunden: $LevelDir"
}

if (-not (Test-Path $MapsDir)) {
    New-Item -Path $MapsDir -ItemType Directory | Out-Null
}

$files = Get-ChildItem -Path $LevelDir -Filter '*.lua' | Sort-Object Name
$roomFiles = $files | Where-Object { $_.Name -ine 'world.lua' }
$worldFile = $files | Where-Object { $_.Name -ieq 'world.lua' } | Select-Object -First 1
$count = 0
$exportedRooms = @{}

foreach ($file in $roomFiles) {
    $outPath = Join-Path $MapsDir ($file.BaseName + '.json')
    Convert-RoomLuaToJson -Path $file.FullName -OutPath $outPath
    Validate-RoomJson -Path $outPath
    $exportedRooms[$file.BaseName] = $true
    $count++
}

if ($null -ne $worldFile) {
    $worldOutPath = Join-Path $MapsDir 'world.json'
    Convert-WorldLuaToJson -Path $worldFile.FullName -OutPath $worldOutPath
    Validate-WorldJson -Path $worldOutPath -AvailableRooms $exportedRooms
    $count++
}

Write-Host "Export fertig. $count Datei(en) nach '$MapsDir' geschrieben."
