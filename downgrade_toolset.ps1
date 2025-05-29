# Script để downgrade từ v143 xuống v142 platform toolset
Write-Host "Bắt đầu downgrade platform toolset từ v143 xuống v142..." -ForegroundColor Green

# Tìm tất cả file .vcxproj
$vcxprojFiles = Get-ChildItem -Path "." -Filter "*.vcxproj" -Recurse

foreach ($file in $vcxprojFiles) {
    Write-Host "Đang xử lý: $($file.FullName)" -ForegroundColor Yellow
    
    # Đọc nội dung file
    $content = Get-Content $file.FullName -Raw
    
    # Kiểm tra xem có v143 không
    if ($content -match "v143") {
        Write-Host "  -> Tìm thấy v143, đang thay đổi thành v142..." -ForegroundColor Cyan
        
        # Thay thế v143 bằng v142
        $newContent = $content -replace "v143", "v142"
        
        # Ghi lại file
        Set-Content -Path $file.FullName -Value $newContent
        
        Write-Host "  -> Đã cập nhật thành công!" -ForegroundColor Green
    } else {
        Write-Host "  -> Không có v143, bỏ qua" -ForegroundColor Gray
    }
}

Write-Host "`nHoàn thành! Tất cả project files đã được downgrade xuống v142." -ForegroundColor Green
Write-Host "Bây giờ bạn có thể build với Visual Studio 2019." -ForegroundColor Green
