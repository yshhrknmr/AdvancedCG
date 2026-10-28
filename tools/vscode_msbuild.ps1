# Builds a .vcxproj with the MSBuild of Visual Studio 2022 (or Build Tools) from Visual Studio Code.
# The post-build event copies the required DLLs next to the executable.
# NOTE: Keep this file ASCII-only. Windows PowerShell 5.1 reads BOM-less files as ANSI (CP932),
#       which breaks parsing if the file contains non-ASCII characters.
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [string]$Target = "Build"
)

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    Write-Error "vswhere.exe not found. Install Visual Studio 2022 or Build Tools for Visual Studio 2022 with the 'Desktop development with C++' workload."
    exit 1
}

$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
if (-not $msbuild) {
    Write-Error "MSBuild.exe not found. Install Visual Studio 2022 or Build Tools for Visual Studio 2022 with the 'Desktop development with C++' workload."
    exit 1
}

& $msbuild $Project /t:$Target /m /nologo /v:minimal "/p:Configuration=$Configuration" "/p:Platform=$Platform"
exit $LASTEXITCODE
