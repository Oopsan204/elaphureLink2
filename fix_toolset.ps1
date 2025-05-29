# PowerShell script to fix platform toolset compatibility
# This script changes v143 to v142 in all .vcxproj files

Write-Host "Fixing Platform Toolset from v143 to v142..." -ForegroundColor Green

# Get all .vcxproj files
$vcxprojFiles = Get-ChildItem -Path "." -Filter "*.vcxproj" -Recurse

foreach ($file in $vcxprojFiles) {
    Write-Host "Processing: $($file.FullName)" -ForegroundColor Yellow
    
    # Read the file content
    $content = Get-Content $file.FullName -Raw
    
    # Replace v143 with v142
    if ($content -match "v143") {
        $newContent = $content -replace "v143", "v142"
        Set-Content -Path $file.FullName -Value $newContent -NoNewline
        Write-Host "  Updated toolset from v143 to v142" -ForegroundColor Cyan
    } else {
        Write-Host "  No v143 toolset found" -ForegroundColor Gray
    }
}

Write-Host "Platform toolset fix completed!" -ForegroundColor Green
Write-Host "Note: You may need to update the GitHub Actions workflow to use windows-2019 if you choose this approach." -ForegroundColor Warning
