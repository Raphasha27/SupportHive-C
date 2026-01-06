$baseDir = Get-Location
$cmakePath = "$baseDir\tools\cmake-3.28.1-windows-x86_64\bin\cmake.exe"
$devkitBin = "$baseDir\tools\w64devkit\w64devkit\bin"

# Add devkit to path for this session
$env:PATH = "$devkitBin;$env:PATH"

# Set compilers
$env:CC = "$devkitBin\gcc.exe"
$env:CXX = "$devkitBin\g++.exe"

if (-not (Test-Path build)) {
    mkdir build
}

cd build

# Use -G "MinGW Makefiles" and point to the make program
&$cmakePath .. -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM="$devkitBin\make.exe"

if ($LASTEXITCODE -eq 0) {
    &$cmakePath --build .
}
