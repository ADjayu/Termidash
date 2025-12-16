# =============================================================================
# Termidash Build Script for Windows (PowerShell)
# =============================================================================
# Usage:
#   .\build.ps1 [command]
#
# Commands:
#   clean      - Remove build directory
#   configure  - Run CMake configuration
#   build      - Build the project (implies configure)
#   test       - Run tests (implies build)
#   package    - Create distribution package (implies build)
#   all        - Run clean, build, test, and package
#   help       - Show this help message
#
# Examples:
#   .\build.ps1 build      # Configure and build
#   .\build.ps1 test       # Build and run tests
#   .\build.ps1 package    # Build and create package
#   .\build.ps1 all        # Full pipeline
# =============================================================================

param(
    [Parameter(Position=0)]
    [ValidateSet("clean", "configure", "build", "test", "package", "all", "help")]
    [string]$Command = "help",
    
    [Parameter()]
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Release"
)

# Configuration
$BuildDir = "build"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Error handling
$ErrorActionPreference = "Stop"

# Helper functions
function Write-Header {
    param([string]$Message)
    Write-Host "=============================================" -ForegroundColor Blue
    Write-Host "  $Message" -ForegroundColor Blue
    Write-Host "=============================================" -ForegroundColor Blue
}

function Write-Success {
    param([string]$Message)
    Write-Host "[OK] $Message" -ForegroundColor Green
}

function Write-Error-Custom {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Yellow
}

# Clean build directory
function Invoke-Clean {
    Write-Header "Cleaning Build Directory"
    
    $buildPath = Join-Path $ScriptDir $BuildDir
    if (Test-Path $buildPath) {
        Remove-Item -Recurse -Force $buildPath
        Write-Success "Removed $BuildDir directory"
    } else {
        Write-Info "Build directory does not exist, nothing to clean"
    }
}

# Configure with CMake
function Invoke-Configure {
    Write-Header "Configuring CMake"
    
    $buildPath = Join-Path $ScriptDir $BuildDir
    if (-not (Test-Path $buildPath)) {
        New-Item -ItemType Directory -Path $buildPath | Out-Null
    }
    
    Write-Info "Build type: $BuildType"
    Write-Info "Build directory: $BuildDir"
    
    Push-Location $ScriptDir
    try {
        cmake -B $BuildDir -DCMAKE_BUILD_TYPE=$BuildType -DBUILD_TESTING=ON
    } finally {
        Pop-Location
    }
    
    Write-Success "CMake configuration complete"
}

# Build the project
function Invoke-Build {
    Write-Header "Building Project"
    
    $cacheFile = Join-Path $ScriptDir $BuildDir "CMakeCache.txt"
    if (-not (Test-Path $cacheFile)) {
        Write-Info "Build not configured, running configure first..."
        Invoke-Configure
    }
    
    # Get number of processors for parallel build
    $jobs = $env:NUMBER_OF_PROCESSORS
    Write-Info "Building with $jobs parallel jobs"
    
    Push-Location $ScriptDir
    try {
        cmake --build $BuildDir --config $BuildType -j $jobs
    } finally {
        Pop-Location
    }
    
    Write-Success "Build complete"
}

# Run tests
function Invoke-Test {
    Write-Header "Running Tests"
    
    $testExe = Join-Path $ScriptDir $BuildDir $BuildType "termidash_tests.exe"
    if (-not (Test-Path $testExe)) {
        Write-Info "Tests not built, running build first..."
        Invoke-Build
    }
    
    Push-Location (Join-Path $ScriptDir $BuildDir)
    try {
        ctest --output-on-failure --build-config $BuildType
    } finally {
        Pop-Location
    }
    
    Write-Success "All tests passed"
}

# Create package
function Invoke-Package {
    Write-Header "Creating Package"
    
    # Build if needed
    Invoke-Build
    
    Push-Location (Join-Path $ScriptDir $BuildDir)
    try {
        Write-Info "Creating Windows ZIP package..."
        cpack -G ZIP -C $BuildType
        
        Write-Success "ZIP package created"
        
        # List created packages
        Write-Info "Packages created:"
        Get-ChildItem -Path . -Filter "*.zip" | ForEach-Object {
            Write-Host "  - $($_.Name)" -ForegroundColor Cyan
        }
    } finally {
        Pop-Location
    }
    
    Write-Success "Package creation complete"
}

# Run all steps
function Invoke-All {
    Invoke-Clean
    Invoke-Build
    Invoke-Test
    Invoke-Package
}

# Show help
function Show-Help {
    Write-Host "Termidash Build Script for Windows" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Usage: .\build.ps1 [command] [-BuildType <Debug|Release>]"
    Write-Host ""
    Write-Host "Commands:"
    Write-Host "  clean      Remove build directory"
    Write-Host "  configure  Run CMake configuration"
    Write-Host "  build      Build the project (implies configure)"
    Write-Host "  test       Run tests (implies build)"
    Write-Host "  package    Create distribution package (implies build)"
    Write-Host "  all        Run clean, build, test, and package"
    Write-Host "  help       Show this help message"
    Write-Host ""
    Write-Host "Parameters:"
    Write-Host "  -BuildType   Set to Debug or Release (default: Release)"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\build.ps1 build                    # Release build"
    Write-Host "  .\build.ps1 build -BuildType Debug   # Debug build"
    Write-Host "  .\build.ps1 test                     # Build and run tests"
    Write-Host "  .\build.ps1 all                      # Full pipeline"
}

# Main entry point
Push-Location $ScriptDir
try {
    switch ($Command) {
        "clean"     { Invoke-Clean }
        "configure" { Invoke-Configure }
        "build"     { Invoke-Build }
        "test"      { Invoke-Test }
        "package"   { Invoke-Package }
        "all"       { Invoke-All }
        "help"      { Show-Help }
        default     { Show-Help }
    }
} catch {
    Write-Error-Custom "Build failed: $_"
    exit 1
} finally {
    Pop-Location
}
