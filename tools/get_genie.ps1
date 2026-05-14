<#
.SYNOPSIS
    Downloads the latest GENie project generator binary for Windows.
.DESCRIPTION
    Downloads genie.exe from the bx repository (maintained by the same author as GENie).
    This binary supports Visual Studio 2022 (vs2022), 2019, 2017, 2015, and MinGW (gmake).
.PARAMETER OutDir
    Directory to save genie.exe. Defaults to the script's parent directory (tools/).
.PARAMETER Force
    Overwrite genie.exe if it already exists.
.EXAMPLE
    .\get_genie.ps1
    Downloads genie.exe to the tools/ directory.
.EXAMPLE
    .\get_genie.ps1 -Force
    Re-downloads even if genie.exe already exists.
#>
param(
    [string]$OutDir = $PSScriptRoot,
    [switch]$Force
)

$genieUrl = "https://github.com/bkaradzic/bx/raw/master/tools/bin/windows/genie.exe"
$outFile = Join-Path $OutDir "genie.exe"

if (Test-Path $outFile -PathType Leaf) {
    if (-not $Force) {
        Write-Host "genie.exe already exists at $outFile" -ForegroundColor Green
        Write-Host "Use -Force to re-download." -ForegroundColor Yellow
        exit 0
    }
    Write-Host "Overwriting existing genie.exe..." -ForegroundColor Yellow
}

Write-Host "Downloading GENie from $genieUrl ..." -ForegroundColor Cyan

try {
    # Try to use BITS if available (more reliable), fall back to Invoke-WebRequest
    if (Get-Command Start-BitsTransfer -ErrorAction SilentlyContinue) {
        Start-BitsTransfer -Source $genieUrl -Destination $outFile
    } else {
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
        Invoke-WebRequest -Uri $genieUrl -OutFile $outFile -UseBasicParsing
    }
    Write-Host "genie.exe downloaded successfully to $outFile" -ForegroundColor Green
} catch {
    Write-Host "ERROR: Failed to download genie.exe: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "Alternative options:" -ForegroundColor Yellow
    Write-Host "1. Build GENie from source: https://github.com/bkaradzic/GENie"
    Write-Host "2. Download manually from: $genieUrl"
    Write-Host "   and place it at: $outFile"
    exit 1
}
