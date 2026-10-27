<#
.SYNOPSIS
Checks that the installed Moonlight executable matches the delivered Axixi build.
.DESCRIPTION
Read-only verification. Does not start Moonlight or read/change user settings.
Use the manifest shipped alongside this script, or pass -ManifestPath explicitly.
An installed executable with the same version but different contents fails.
#>
[CmdletBinding()]
param(
    [string]$InstallDirectory = (Join-Path $env:ProgramFiles 'Moonlight Game Streaming'),
    [string]$ManifestPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (!$ManifestPath) {
    $ManifestPath = Join-Path $PSScriptRoot 'Moonlight-install-manifest.json'
    if (!(Test-Path -LiteralPath $ManifestPath -PathType Leaf)) {
        $ManifestPath = Join-Path $PSScriptRoot '../build/artifacts/Moonlight-install-manifest.json'
    }
}

$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
if ($manifest.schemaVersion -ne 1 -or $manifest.executableSha256 -notmatch '^[0-9a-fA-F]{64}$') {
    throw 'Invalid installation verification manifest.'
}

$executable = Join-Path $InstallDirectory 'Moonlight.exe'
if (!(Test-Path -LiteralPath $executable -PathType Leaf)) {
    throw "Moonlight.exe was not found in: $InstallDirectory"
}

$actualHash = (Get-FileHash -LiteralPath $executable -Algorithm SHA256).Hash
if ($actualHash -ne $manifest.executableSha256) {
    throw "Installed Moonlight.exe does not match this Axixi build. Expected SHA256: $($manifest.executableSha256); actual: $actualHash. Run the updated installer and check again."
}

[pscustomobject]@{
    Result = 'PASS - installed executable matches the Axixi build'
    Executable = (Resolve-Path -LiteralPath $executable).Path
    Version = (Get-Item -LiteralPath $executable).VersionInfo.FileVersion
    SHA256 = $actualHash
}
