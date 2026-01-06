# SupportHive-C Installation Guide

## Prerequisites Installation

I've initiated downloads for both required tools:

### 1. CMake (Build System)
- **File**: `cmake-4.2.1-windows-x86_64.msi`
- **Installation**: 
  1. Run the downloaded `.msi` installer
  2. ✅ **IMPORTANT**: During installation, select "Add CMake to the system PATH for all users"
  3. Complete the installation wizard

### 2. MinGW-w64 (C Compiler via MSYS2)
- **File**: `msys2-x86_64-20251213.exe`
- **Installation**:
  1. Run the downloaded `.exe` installer
  2. Install to the default location (usually `C:\msys64`)
  3. After installation completes, MSYS2 will open a terminal automatically
  4. In the MSYS2 terminal, run these commands:
     ```bash
     pacman -Syu
     # Close the terminal when prompted, then reopen MSYS2
     pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
     ```
  5. Add MinGW to your PATH:
     - Open Windows Settings → System → About → Advanced system settings
     - Click "Environment Variables"
     - Under "System variables", find and edit "Path"
     - Add: `C:\msys64\mingw64\bin`
     - Click OK to save

### 3. Verify Installation
After installing both tools, **restart your PowerShell** and run:
```powershell
cmake --version
gcc --version
```

Both commands should now work without errors.

## Building SupportHive-C

Once the prerequisites are installed:

```powershell
cd c:\Users\rapha\OneDrive\Desktop\SupportHive-C
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## Running the Engine

```powershell
.\supporthive.exe
```

You should see:
```
🐝 SupportHive-C engine listening on port 7000
```

## Testing the Server

In another terminal:
```powershell
curl http://localhost:7000
```

Expected response:
```
HTTP/1.1 200 OK
Content-Type: text/plain

SupportHive-C Engine Operational
```

## Next Steps After Installation

Choose one to continue building:
1. **HTTP Request Router** - Parse actual HTTP requests using http-parser
2. **PostgreSQL Integration** - Connect to database for ticket storage
3. **SLA Timer Engine** - Event-driven timer system using libuv timers
4. **Tenant Isolation Layer** - Multi-tenant routing and data scoping

---

**Note**: Make sure to restart your terminal/PowerShell after installing CMake and MinGW so the PATH changes take effect.
