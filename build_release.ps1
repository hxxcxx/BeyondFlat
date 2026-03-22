# Build script for BeyondLinear (Release configuration)

Write-Host "Building BeyondLinear (Release)..." -ForegroundColor Green

# Create build directory if it doesn't exist
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build"
}

# Configure and build
Set-Location build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Return to root directory
Set-Location ..

Write-Host "Build complete! Executable is at: build\bin\Release\linear_viewer.exe" -ForegroundColor Green
