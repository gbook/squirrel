[CmdletBinding(PositionalBinding = $false)]
param(
    [ValidateSet('Release', 'Debug')]
    [string]$Config = 'Release',
    [string]$ExePath,
    [string]$QtBin,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ProgramArgs
)

$ErrorActionPreference = 'Stop'

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$SourceRoot = Join-Path $ProjectRoot 'src\squirrel'

function Resolve-SquirrelExe {
    param(
        [string]$Configuration,
        [string]$ExplicitPath
    )

    if ($ExplicitPath) {
        if (-not (Test-Path -LiteralPath $ExplicitPath)) {
            throw "Executable not found: $ExplicitPath"
        }
        return (Resolve-Path -LiteralPath $ExplicitPath).Path
    }

    $candidates = switch ($Configuration) {
        'Debug' {
            @(
                (Join-Path $SourceRoot 'build\Desktop_Qt_6_9_3_MSVC2022_64bit-Debug\debug\squirrel.exe'),
                (Join-Path $SourceRoot 'build\Desktop_Qt_6_9_1_MSVC2022_64bit-Debug\debug\squirrel.exe'),
                (Join-Path $SourceRoot 'build\Desktop_Qt_6_8_2_MSVC2022_64bit-Debug\debug\squirrel.exe')
            )
        }
        default {
            @(
                (Join-Path $SourceRoot 'build\Desktop_Qt_6_9_3_MSVC2022_64bit-Release\release\squirrel.exe'),
                (Join-Path $SourceRoot 'build\Desktop_Qt_6_9_1_MSVC2022_64bit-Release\release\squirrel.exe'),
                (Join-Path $SourceRoot 'build\Desktop_Qt_6_8_2_MSVC2022_64bit-Release\release\squirrel.exe')
            )
        }
    }

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "Could not find a squirrel.exe for configuration '$Configuration'. Use -ExePath to specify it explicitly."
}

function Resolve-QtBin {
    param(
        [string]$ExplicitQtBin,
        [string]$ExecutablePath
    )

    if ($ExplicitQtBin) {
        if (-not (Test-Path -LiteralPath $ExplicitQtBin)) {
            throw "Qt bin path not found: $ExplicitQtBin"
        }
        return (Resolve-Path -LiteralPath $ExplicitQtBin).Path
    }

    $exePathNormalized = $ExecutablePath.Replace('/', '\')
    $qtVersion = if ($exePathNormalized -match 'Desktop_Qt_([^_]+)_MSVC2022') {
        $matches[1] -replace '_', '.'
    }
    else {
        '6.9.3'
    }

    $candidates = @(
        "C:\Qt\$qtVersion\msvc2022_64\bin",
        'C:\Qt\6.9.3\msvc2022_64\bin',
        'C:\Qt\6.9.1\msvc2022_64\bin',
        'C:\Qt\6.8.2\msvc2022_64\bin'
    )

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "Could not find a Qt msvc2022_64 bin directory. Use -QtBin to specify it explicitly."
}

$resolvedExe = Resolve-SquirrelExe -Configuration $Config -ExplicitPath $ExePath
$resolvedQtBin = Resolve-QtBin -ExplicitQtBin $QtBin -ExecutablePath $resolvedExe
$exeDir = Split-Path -Parent $resolvedExe

$env:PATH = (($exeDir, $resolvedQtBin, $env:PATH) -join ';')

Write-Host "Launching: $resolvedExe"
Write-Host "Using Qt bin: $resolvedQtBin"

& $resolvedExe @ProgramArgs
exit $LASTEXITCODE
