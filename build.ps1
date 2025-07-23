New-Item -Path "build" -ItemType Directory -Force > $null

Set-Location -Path "build"

cmake .

cmake --build .

Set-Location -Path ".."