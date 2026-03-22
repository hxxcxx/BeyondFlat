# Build script for BeyondLinear (RelWithDebInfo configuration)

Write-Host "Building BeyondLinear (RelWithDebInfo)..." -ForegroundColor Green

# Create build directory if it doesn't exist
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build"
}

# Configure and build
Set-Location build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --config RelWithDebInfo

# Return to root directory
Set-Location ..

Write-Host "Build complete! Executable is at: build\bin\RelWithDebInfo\linear_viewer.exe" -ForegroundColor Green
