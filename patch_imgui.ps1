param(
    [string]$ImguiRecipePath
)

if (-not $ImguiRecipePath) {
    Write-Host "Usage: patch_imgui.ps1 <path-to-imgui-xmake.lua>"
    exit 1
}

if (-not (Test-Path $ImguiRecipePath)) {
    Write-Host "ERROR: File not found: $ImguiRecipePath"
    exit 1
}

$portPath = Join-Path (Split-Path $ImguiRecipePath) "port/xmake.lua"
if (-not (Test-Path $portPath)) {
    Write-Host "ERROR: Port file not found: $portPath"
    exit 1
}

Write-Host "  Patching imgui: use NO_PROTOTYPES for vulkan=true..."

$content = [System.IO.File]::ReadAllText($portPath)

# Check if the vulkan=true block already has the NO_PROTOTYPES define
# We need to check AFTER "elseif has_config(""vulkan"") then" but BEFORE the next "end"
if ($content -match 'elseif has_config\("vulkan"\).*?IMGUI_IMPL_VULKAN_NO_PROTOTYPES') {
    Write-Host "  Already patched, skip."
    exit 0
}

# Fix: In the vulkan=true block (NOT vulkan_no_proto), add NO_PROTOTYPES define
# and remove any stray add_linkdirs/add_links for vulkan-1.lib
#
# Target pattern (original):
#        elseif has_config("vulkan") then
#            add_packages("vulkan-headers")
#        end
#    end
#
# Replace with:
#        elseif has_config("vulkan") then
#            add_packages("vulkan-headers")
#            add_defines("IMGUI_IMPL_VULKAN_NO_PROTOTYPES")
#        end
#    end

# Pattern matches from "elseif has_config(""vulkan"")" to the two "end"s
# This handles any garbage (add_linkdirs, add_links, etc.) between the original add_packages and end
$pattern = '(elseif\s+has_config\("vulkan"\)\s+then\s+add_packages\("vulkan-headers"\))(.*?)(\s+end\s+end)'

# Check if pattern matched
if ($content -notmatch $pattern) {
    Write-Host "  WARNING: Could not find vulkan=true block in port file."
    exit 1
}

$newContent = $content -replace $pattern, ('${1}' + "`r`n" + '            add_defines("IMGUI_IMPL_VULKAN_NO_PROTOTYPES")${3}')

if ($newContent -eq $content) {
    Write-Host "  WARNING: No changes made."
    exit 1
}

[System.IO.File]::WriteAllText($portPath, $newContent)

# Verify
$verifyContent = [System.IO.File]::ReadAllText($portPath)
if ($verifyContent -match 'elseif has_config\("vulkan"\).*?IMGUI_IMPL_VULKAN_NO_PROTOTYPES') {
    Write-Host "  Patch applied successfully."
    exit 0
} else {
    Write-Host "  WARNING: Verification failed."
    exit 1
}
