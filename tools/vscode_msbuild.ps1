# Visual Studio Code から Visual Studio 2022 (または Build Tools) の MSBuild を呼び出して
# .vcxproj をビルドするためのスクリプトです。
# ビルド後イベントにより、必要な DLL は実行ファイルと同じフォルダに自動でコピーされます。
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [string]$Target = "Build"
)

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    Write-Error "vswhere.exe が見つかりません。Visual Studio 2022 (C++ によるデスクトップ開発) をインストールしてください。"
    exit 1
}

$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
if (-not $msbuild) {
    Write-Error "MSBuild.exe が見つかりません。Visual Studio 2022 (C++ によるデスクトップ開発) をインストールしてください。"
    exit 1
}

& $msbuild $Project /t:$Target /m /nologo /v:minimal "/p:Configuration=$Configuration" "/p:Platform=$Platform"
exit $LASTEXITCODE
