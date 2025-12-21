# Codex CLI Installation Script (Windows PowerShell)
# Usage: Run PowerShell as Administrator
# Example: .\install-codex-windows.ps1 -BaseUrl "https://codex.heihuzicity.com/openai" -ApiKey "cr_xxxxxxxxxx"

param(
    [string]$BaseUrl = "https://codex.heihuzicity.com/openai",
    [string]$ApiKey = ""
)

$ErrorActionPreference = "Continue"

Write-Host ""
Write-Host "========================================" -ForegroundColor Magenta
Write-Host "   Codex CLI Installation (Windows)" -ForegroundColor Magenta
Write-Host "========================================" -ForegroundColor Magenta
Write-Host ""

# Check if running as Administrator
$currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
$isAdmin = $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "[WARN] Recommend running as Administrator to avoid permission issues" -ForegroundColor Yellow
}

# 1. Check Node.js
Write-Host "[INFO] Checking Node.js..." -ForegroundColor Cyan
$nodeVersion = $null
$nodeVersion = & node --version 2>$null

if ($nodeVersion) {
    $versionMatch = [regex]::Match($nodeVersion, 'v(\d+)')
    if ($versionMatch.Success) {
        $versionNum = [int]$versionMatch.Groups[1].Value
        if ($versionNum -ge 18) {
            Write-Host "[OK] Node.js installed: $nodeVersion" -ForegroundColor Green
        }
        else {
            Write-Host "[ERROR] Node.js version too low ($nodeVersion), requires v18.x or higher" -ForegroundColor Red
            Write-Host "[INFO] Please visit https://nodejs.org/ to download the latest LTS version" -ForegroundColor Cyan
            exit 1
        }
    }
}
else {
    Write-Host "[ERROR] Node.js not detected" -ForegroundColor Red
    Write-Host "[INFO] Attempting to install Node.js automatically..." -ForegroundColor Cyan
    
    $hasWinget = Get-Command winget -ErrorAction SilentlyContinue
    if ($hasWinget) {
        Write-Host "[INFO] Installing Node.js via winget..." -ForegroundColor Cyan
        & winget install OpenJS.NodeJS.LTS --accept-package-agreements --accept-source-agreements
        
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
        
        $nodeVersion = & node --version 2>$null
        if ($nodeVersion) {
            Write-Host "[OK] Node.js installed: $nodeVersion" -ForegroundColor Green
        }
        else {
            Write-Host "[ERROR] Node.js not detected after installation, please reopen PowerShell and run this script again" -ForegroundColor Red
            exit 1
        }
    }
    else {
        Write-Host "[ERROR] winget not found, please install Node.js manually" -ForegroundColor Red
        Write-Host "[INFO] Download: https://nodejs.org/" -ForegroundColor Cyan
        exit 1
    }
}

# 2. Check npm
Write-Host "[INFO] Checking npm..." -ForegroundColor Cyan
$npmVersion = & npm --version 2>$null
if ($npmVersion) {
    Write-Host "[OK] npm installed: v$npmVersion" -ForegroundColor Green
}
else {
    Write-Host "[ERROR] npm not found, please reinstall Node.js" -ForegroundColor Red
    exit 1
}

# 3. Install Codex CLI
Write-Host "[INFO] Installing Codex CLI..." -ForegroundColor Cyan
$codexVersion = & codex --version 2>$null

if ($codexVersion) {
    Write-Host "[OK] Codex CLI already installed: $codexVersion" -ForegroundColor Green
    Write-Host "[INFO] Updating to latest version..." -ForegroundColor Cyan
}

# Get npm global path before install
$npmGlobalPath = & npm config get prefix
Write-Host "[INFO] npm global path: $npmGlobalPath" -ForegroundColor Cyan

$ErrorActionPreference = "SilentlyContinue"
& npm install -g @openai/codex 2>&1 | Out-Null
$installExitCode = $LASTEXITCODE
$ErrorActionPreference = "Continue"

if ($installExitCode -ne 0) {
    Write-Host "[WARN] npm global install may have permission issues, trying user directory..." -ForegroundColor Yellow
    $npmGlobalPath = "$env:APPDATA\npm"
    & npm config set prefix $npmGlobalPath
    Write-Host "[INFO] Changed npm global path to: $npmGlobalPath" -ForegroundColor Cyan
    $ErrorActionPreference = "SilentlyContinue"
    & npm install -g @openai/codex 2>&1 | Out-Null
    $ErrorActionPreference = "Continue"
}

# Refresh Path to include npm global bin (Windows npm puts binaries directly in prefix folder)
$env:Path = "$npmGlobalPath;$env:Path"

# Also add to user PATH permanently
$userPath = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::User)
if ($userPath -notlike "*$npmGlobalPath*") {
    [System.Environment]::SetEnvironmentVariable("Path", "$npmGlobalPath;$userPath", [System.EnvironmentVariableTarget]::User)
    Write-Host "[OK] Added npm global path to user PATH" -ForegroundColor Green
}

$codexVersion = & codex --version 2>$null
if ($codexVersion) {
    Write-Host "[OK] Codex CLI installed: $codexVersion" -ForegroundColor Green
}
else {
    Write-Host "[ERROR] Codex CLI installation failed" -ForegroundColor Red
    Write-Host "[INFO] Try running: npm install -g @openai/codex" -ForegroundColor Cyan
    Write-Host "[INFO] Then add npm global path to your PATH environment variable" -ForegroundColor Cyan
    exit 1
}

# 4. Get parameters
Write-Host ""
Write-Host "[INFO] Current configuration:" -ForegroundColor Cyan
Write-Host "  Base URL: $BaseUrl"

if (-not $ApiKey) {
    Write-Host "[INFO] Please enter your API key (format: cr_xxxxxxxxxx)" -ForegroundColor Cyan
    Write-Host "[INFO] If you don't have a key, visit https://codex.heihuzicity.com to get one" -ForegroundColor Cyan
    $ApiKey = Read-Host "API Key"
}

if (-not $ApiKey) {
    Write-Host "[WARN] No API Key provided, skipping environment variable configuration" -ForegroundColor Yellow
    Write-Host "[INFO] You can manually set environment variable CRS_OAI_KEY later" -ForegroundColor Cyan
}
else {
    Write-Host "[INFO] Configuring environment variable..." -ForegroundColor Cyan
    [System.Environment]::SetEnvironmentVariable("CRS_OAI_KEY", $ApiKey, [System.EnvironmentVariableTarget]::User)
    $env:CRS_OAI_KEY = $ApiKey
    Write-Host "[OK] Environment variable CRS_OAI_KEY set" -ForegroundColor Green
}

# 5. Create configuration files
Write-Host "[INFO] Creating configuration files..." -ForegroundColor Cyan
$codexDir = "$env:USERPROFILE\.codex"
if (-not (Test-Path $codexDir)) {
    New-Item -ItemType Directory -Path $codexDir -Force | Out-Null
}

$configToml = @"
model_provider = "crs"
model = "gpt-5-codex"
model_reasoning_effort = "high"
disable_response_storage = true
preferred_auth_method = "apikey"

[model_providers.crs]
name = "crs"
base_url = "$BaseUrl"
wire_api = "responses"
requires_openai_auth = true
env_key = "CRS_OAI_KEY"
"@

$configPath = "$codexDir\config.toml"
if (Test-Path $configPath) {
    $timestamp = Get-Date -Format "yyyyMMddHHmmss"
    $backup = "$configPath.backup.$timestamp"
    Copy-Item $configPath $backup
    Write-Host "[INFO] Backed up original config to: $backup" -ForegroundColor Cyan
}
[System.IO.File]::WriteAllText($configPath, $configToml, [System.Text.UTF8Encoding]::new($false))
Write-Host "[OK] Config file created: $configPath" -ForegroundColor Green

$authJson = @"
{
  "OPENAI_API_KEY": null
}
"@

$authPath = "$codexDir\auth.json"
[System.IO.File]::WriteAllText($authPath, $authJson, [System.Text.UTF8Encoding]::new($false))
Write-Host "[OK] Auth file created: $authPath" -ForegroundColor Green

# 6. Done
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "   Installation Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "[INFO] Config location: $codexDir" -ForegroundColor Cyan
Write-Host "[INFO] Environment variable: CRS_OAI_KEY" -ForegroundColor Cyan
Write-Host ""
Write-Host "Usage:" -ForegroundColor Yellow
Write-Host "  1. Reopen PowerShell (to apply environment variables)"
Write-Host "  2. Navigate to project: cd your-project"
Write-Host "  3. Start Codex: codex"
Write-Host ""
