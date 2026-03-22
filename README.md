# BeyondFlat

A CAGD (Computer Aided Geometric Design) learning project, focusing on curves and surfaces. Companion to BeyondConvex.

## Project Structure

```
BeyondFlat/
├── src/              # Linear algebra library source code
├── viewer/           # Visualization viewer
├── third_party/      # Third-party dependencies (GLFW, ImGui)
├── docs/            # Documentation
└── build/           # Build output directory
```

## Building

### Windows (PowerShell)

```powershell
# Create build directory
mkdir build
cd build

# Configure with CMake (Release)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

### Running

After building, the viewer executable will be in `build/bin/Release/`:

```powershell
.\build\bin\Release\linear_viewer.exe
```

## Dependencies

- **GLFW**: Window and input management
- **ImGui**: Immediate mode GUI
- **OpenGL**: Graphics rendering

## Features (To Be Implemented)

- Vector operations
- Matrix operations
- Linear solvers
- Visualization tools
- Interactive demos

## License

See LICENSE file for details.
