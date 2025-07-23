New-Item -Path "build" -ItemType Directory -Force

Set-Location -Path "build"

cmake .

cmake --build .

Set-Location -Path ".."