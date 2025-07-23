New-Item -Path "build" -ItemType Directory -Force > $null

$OriginalLocation = Get-Location

Set-Location $PSScriptRoot\build

cmake .

cmake --build .

Set-Location $OriginalLocation