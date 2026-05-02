# 🛠️ SupportHive-C Installation Guide

This guide covers how to build and run the SupportHive-C engine using the portable toolchain included in the project.

## 🚀 One-Step Build (Windows)

The project includes a portable build system using CMake and GCC (MinGW-w64). No system-wide installations are required.

### 1. Build the Engine
Run the included PowerShell script to configure and compile the project:
```powershell
./build_portable.ps1
```

### 2. Run the Engine
Once the build completes, start the server:
```powershell
$env:PATH = "$PWD\tools\w64devkit\w64devkit\bin;$env:PATH"
./build/supporthive.exe
```

## 📱 Mobile Review (Android / iOS)

1. **Get your Local IP** (visible only to you — never share or commit this value):
   - **Windows**: Run `ipconfig` in PowerShell → look for **IPv4 Address** under your active Wi-Fi adapter.
   - **Mac/Linux**: Run `ifconfig | grep "inet "` in Terminal.

2. **Connect on your mobile device**:
   Open your browser and navigate to:
   ```
   http://<YOUR_LOCAL_IP>:7000/dashboard
   ```
   > ⚠️ Your local IP is private network data. Never hardcode it in any file that gets committed to a repository.

## 🧪 Quick Test

Check if the engine is responding using PowerShell:
```powershell
Invoke-RestMethod -Uri http://localhost:7000/ -Method Get
```

Expected Response:
```json
{
  "service": "SupportHive-C",
  "version": "1.0.0"
}
```

## 🏗️ Manual Build (Advanced)
If you prefer using your own system tools:
1. Ensure `cmake` and `gcc` are in your PATH.
2. Initialize build:
   ```bash
   mkdir build && cd build
   cmake .. -G "MinGW Makefiles"
   cmake --build .
   ```

---
*SupportHive-C: Engineering Excellence in Systems Programming.*
