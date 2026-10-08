param(
    [int]$subTask = 1,
    [int]$ithTest = 0  # 預設為 0 代表測試全部；若指定數字 (如 3) 則只測試該編號
)

# 1. 檢查輸入資料夾是否存在
$inFolder = "stress${subTask}"
if (-not (Test-Path $inFolder)) {
    Write-Host "找不到資料夾 $inFolder！" -ForegroundColor Red
    exit
}

# 2. 自動檢查並建立輸出資料夾
$outFolder = "myout${subTask}"
if (-not (Test-Path $outFolder)) {
    New-Item -Path $outFolder -ItemType Directory | Out-Null
}

# 3. 抓取 .in 檔，並根據 $ithTest 決定是抓取全部還是特定檔案
if ($ithTest -gt 0) {
    # 只抓取特定的 ${ithTest}.in
    $testFiles = Get-ChildItem -Path $inFolder -Filter "${ithTest}.in"
}
else {
    # 自動抓取所有數字命名的 .in 檔並由小到大排序
    $testFiles = Get-ChildItem -Path $inFolder -Filter "*.in" | 
    Where-Object { $_.BaseName -match '^\d+$' } | 
    Sort-Object { [int]$_.BaseName }
}

if ($testFiles.Count -eq 0) {
    if ($ithTest -gt 0) {
        Write-Host "在 $inFolder 中找不到指定的測試檔案 (${ithTest}.in)。" -ForegroundColor Yellow
    }
    else {
        Write-Host "在 $inFolder 中找不到任何數字命名的測試檔案 (如 1.in, 2.in)。" -ForegroundColor Yellow
    }
    exit
}

if ($ithTest -gt 0) {
    Write-Host "Subtask $subTask - 開始單獨測試 ${ithTest}.in..." -ForegroundColor Cyan
}
else {
    Write-Host "在 Subtask $subTask 找到 $($testFiles.Count) 個測資，開始測試..." -ForegroundColor Cyan
}

# 4. 針對自動找到的每一個 $i 進行測試
foreach ($file in $testFiles) {
    [int]$i = $file.BaseName
    $inPath = "stress${subTask}\${i}.in"
    $ansPath = "stress${subTask}\${i}.ans"
    $outPath = "${outFolder}\${i}.out"

    if (-not (Test-Path $inPath)) {
        Write-Host "找不到測資 $inPath，測試結束。" -ForegroundColor Yellow
        break
    }
    
    # 執行程式並將輸入傳入
    Get-Content $inPath | & ".\prob${subTask}.exe" > $outPath

    # 比對輸出
    $diff = Compare-Object (Get-Content $outPath) (Get-Content $ansPath)
    if ($null -eq $diff) {
        Write-Host "Test ${i}: AC" -ForegroundColor Green
    }
    else {
        Write-Host "Test ${i}: WA" -ForegroundColor Red
        # Write-Host "=== Input ==="
        # Get-Content $inPath
        # Write-Host "=== Your Output ==="
        # Get-Content $outPath
        # Write-Host "=== Expected Output ==="
        # Get-Content $ansPath
        # break
    }
}