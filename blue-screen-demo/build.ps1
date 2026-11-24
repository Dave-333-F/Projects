$ErrorActionPreference = 'Stop'
$compilerCommand = Get-Command g++ -ErrorAction SilentlyContinue
$compilerPath = if ($compilerCommand) { $compilerCommand.Source } else { 'C:\msys64\ucrt64\bin\g++.exe' }
if (-not (Test-Path -LiteralPath $compilerPath)) { throw 'An existing C++17 compiler is required.' }
$destination = Join-Path $PSScriptRoot '..\bin'
New-Item -ItemType Directory -Path $destination -Force | Out-Null
$previousCompilerPath = $env:PATH
try {
    $env:PATH = (Split-Path -Parent $compilerPath) + ';' + $env:PATH
    & $compilerPath '-std=c++17' '-Wall' '-Wextra' '-Wpedantic' '-O2' '-static' '-municode' '-mwindows' `
        (Join-Path $PSScriptRoot 'main.cpp') '-o' (Join-Path $destination 'blue-screen-demo.exe') '-lgdi32' '-luser32'
    if ($LASTEXITCODE -ne 0) { throw 'Blue-screen demo build failed. Close its existing window before rebuilding.' }
} finally { $env:PATH = $previousCompilerPath }
Write-Output "Built: $destination\blue-screen-demo.exe"
