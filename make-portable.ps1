# Build Portable ZIP and Installer Script
# Run this to create Durandal-Portable.zip

$ErrorActionPreference = "Stop"

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ReleaseDir = Join-Path $ProjectDir "release"
$OutputDir = Join-Path $ProjectDir ".."

if (-not (Test-Path $ReleaseDir)) {
    Write-Host "Error: release/ folder not found at $ReleaseDir" -ForegroundColor Red
    exit 1
}

$ZipPath = Join-Path $OutputDir "Durandal-v1.0.0-Windows-x64.zip"

Write-Host "Creating portable ZIP: $ZipPath"
if (Test-Path $ZipPath) { Remove-Item $ZipPath }

Compress-Archive -Path "$ReleaseDir\*" -DestinationPath $ZipPath -CompressionLevel Optimal

$Size = (Get-Item $ZipPath).Length / 1MB
Write-Host "Done! Size: $([math]::Round($Size, 2)) MB" -ForegroundColor Green
Write-Host "Location: $ZipPath"
