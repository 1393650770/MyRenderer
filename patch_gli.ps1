param(
    [string]$GliRecipePath
)

if (-not $GliRecipePath) {
    Write-Host "Usage: patch_gli.ps1 <path-to-gli-xmake.lua>"
    exit 1
}

if (-not (Test-Path $GliRecipePath)) {
    Write-Host "ERROR: File not found: $GliRecipePath"
    exit 1
}

Write-Host "  Patching gli recipe for GLM 1.0.3 compatibility..."

$content = [System.IO.File]::ReadAllText($GliRecipePath)

# Check if already patched
if ($content -match 'gli::make_vec4') {
    Write-Host "  Already patched, skip."
    exit 0
}

# Replace os.rm("gli/CMakeLists.txt") with io.gsub(...) + original line
$pattern = 'os\.rm\("gli/CMakeLists\.txt"\)'
$replacement = 'io.gsub("gli/core/convert_func.hpp", "return make_vec4<", "return gli::make_vec4<")' + "`r`n        " + 'os.rm("gli/CMakeLists.txt")'

$newContent = $content -replace $pattern, $replacement

if ($newContent -eq $content) {
    Write-Host "  WARNING: Pattern not found, file unchanged."
    exit 1
}

[System.IO.File]::WriteAllText($GliRecipePath, $newContent)
Write-Host "  Patch applied successfully."
exit 0
