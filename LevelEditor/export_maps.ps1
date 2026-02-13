param(
    [string]$LevelDir = "$PSScriptRoot\level",
    [string]$MapsDir = "$PSScriptRoot\..\3ds-cpp\romfs\maps"
)

$ErrorActionPreference = 'Stop'

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

    $tileSize = 16.0
    $slopeMap = @{
        'slope_ur' = 30
        'slope_ul' = 31
        'slope_dr' = 32
        'slope_dl' = 33
    }

    $objectMatches = [regex]::Matches($content, '\{[^{}]*x\s*=\s*(-?\d+(?:\.\d+)?)\s*,\s*y\s*=\s*(-?\d+(?:\.\d+)?)\s*,\s*w\s*=\s*(-?\d+(?:\.\d+)?)\s*,\s*h\s*=\s*(-?\d+(?:\.\d+)?)\s*,\s*type\s*=\s*(\d+)\s*,\s*shape\s*=\s*"([^"]+)"[^{}]*\}')
    foreach ($m in $objectMatches) {
        $shape = [string]$m.Groups[6].Value
        if (-not $slopeMap.ContainsKey($shape)) { continue }
        if ([int]$m.Groups[5].Value -ne 1) { continue }

        $objX = [double]$m.Groups[1].Value
        $objY = [double]$m.Groups[2].Value
        $objW = [double]$m.Groups[3].Value
        $objH = [double]$m.Groups[4].Value
        if ($objW -le 0 -or $objH -le 0) { continue }

        $xStart = [int][Math]::Floor($objX / $tileSize)
        $yStart = [int][Math]::Floor($objY / $tileSize)
        $xEnd = [int][Math]::Ceiling(($objX + $objW) / $tileSize) - 1
        $yEnd = [int][Math]::Ceiling(($objY + $objH) / $tileSize) - 1

        $coverage = @{}
        for ($tx = $xStart; $tx -le $xEnd; $tx++) {
            $coveredRows = New-Object System.Collections.Generic.List[int]
            for ($ty = $yStart; $ty -le $yEnd; $ty++) {
                if ($ty -lt 0 -or $ty -ge $rows.Count) { continue }
                if ($tx -lt 0 -or $tx -ge $rows[$ty].Count) { continue }

                $centerX = ($tx + 0.5) * $tileSize
                $centerY = ($ty + 0.5) * $tileSize
                $u = ($centerX - $objX) / $objW
                $v = ($centerY - $objY) / $objH

                if ($u -lt 0.0 -or $u -gt 1.0 -or $v -lt 0.0 -or $v -gt 1.0) { continue }

                $fill = $false
                if ($shape -eq 'slope_ur') {
                    $fill = $v -ge (1.0 - $u)
                } elseif ($shape -eq 'slope_ul') {
                    $fill = $v -ge $u
                } elseif ($shape -eq 'slope_dr') {
                    $fill = $v -le (1.0 - $u)
                } elseif ($shape -eq 'slope_dl') {
                    $fill = $v -le $u
                }

                if ($fill) {
                    [void]$coveredRows.Add($ty)
                }
            }

            if ($coveredRows.Count -eq 0) { continue }
            $coverage[$tx] = $coveredRows
        }

        foreach ($entry in $coverage.GetEnumerator()) {
            $tx = [int]$entry.Key
            $ys = @($entry.Value)
            if ($ys.Count -eq 0) { continue }

            if ($shape -eq 'slope_ur' -or $shape -eq 'slope_ul') {
                $surfaceY = ($ys | Measure-Object -Minimum).Minimum
                foreach ($ty in $ys) {
                    if ($tx -lt 0 -or $ty -lt 0 -or $ty -ge $rows.Count -or $tx -ge $rows[$ty].Count) { continue }
                    if ($rows[$ty][$tx] -ne 0) { continue }

                    if ($ty -eq $surfaceY) {
                        $rows[$ty][$tx] = [int]$slopeMap[$shape]
                    } else {
                        $rows[$ty][$tx] = 1
                    }
                }
            } else {
                $surfaceY = ($ys | Measure-Object -Maximum).Maximum
                foreach ($ty in $ys) {
                    if ($tx -lt 0 -or $ty -lt 0 -or $ty -ge $rows.Count -or $tx -ge $rows[$ty].Count) { continue }
                    if ($rows[$ty][$tx] -ne 0) { continue }

                    if ($ty -eq $surfaceY) {
                        $rows[$ty][$tx] = [int]$slopeMap[$shape]
                    } else {
                        $rows[$ty][$tx] = 1
                    }
                }
            }
        }
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
