param(
    [int]$subTask = 1,
    [string]$ithTest = ""  # 預設為空字串代表測試全部；若指定檔名主體 (如 "3" 或 "sample1") 則只測試該檔案
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
if ($ithTest -ne "") {
    # 只抓取特定的 ${ithTest}.in
    $testFiles = Get-ChildItem -Path $inFolder -Filter "${ithTest}.in"
}
else {
    # 自動抓取所有的 .in 檔並進行自然數字排序（如 2.in 會排在 10.in 前面，且支援非數字檔名）
    $testFiles = Get-ChildItem -Path $inFolder -Filter "*.in" | 
    Sort-Object { [regex]::Replace($_.Name, '\d+', { $args[0].Value.PadLeft(10, '0') }) }
}

if ($testFiles.Count -eq 0) {
    if ($ithTest -ne "") {
        Write-Host "在 $inFolder 中找不到指定的測試檔案 (${ithTest}.in)。" -ForegroundColor Yellow
    }
    else {
        Write-Host "在 $inFolder 中找不到任何 .in 測試檔案。" -ForegroundColor Yellow
    }
    exit
}

if ($ithTest -ne "") {
    Write-Host "Subtask $subTask - 開始單獨測試 ${ithTest}.in..." -ForegroundColor Cyan
}
else {
    Write-Host "在 Subtask $subTask 找到 $($testFiles.Count) 個測資，開始測試..." -ForegroundColor Cyan
}

# 4. 針對自動找到的每一個檔案進行測試
foreach ($file in $testFiles) {
    $baseName = $file.BaseName
    $inPath   = $file.FullName
    $ansPath  = "${inFolder}\${baseName}.out"
    $outPath  = "${outFolder}\${baseName}.out"

    if (-not (Test-Path $inPath)) {
        Write-Host "找不到測資 $inPath，測試結束。" -ForegroundColor Yellow
        break
    }

    if (-not (Test-Path $ansPath)) {
        Write-Host "警告：找不到答案檔 $ansPath，跳過此測資。" -ForegroundColor Yellow
        continue
    }
    
    # 執行程式並將輸入傳入
    Get-Content $inPath | & ".\prob${subTask}.exe" | Out-File -FilePath $outPath -Encoding ascii

    # 檢查輸出檔是否存在以及長度是否為 0（空檔）
    if ((-not (Test-Path $outPath)) -or ((Get-Item $outPath).Length -eq 0)) {
        Write-Host "Test ${baseName}: WA (輸出檔為空檔，程式可能崩潰或未印出任何內容)" -ForegroundColor Red
        continue
    }

    # 安全地讀取內容（避免空檔回傳 $null 導致 Compare-Object 報錯）
    $outContent = @(Get-Content $outPath)
    $ansContent = @(Get-Content $ansPath)

    # 比對輸出
    $diff = Compare-Object $outContent $ansContent
    if ($null -eq $diff) {
        Write-Host "Test ${baseName}: AC" -ForegroundColor Green
    }
    else {
        Write-Host "Test ${baseName}: WA" -ForegroundColor Red
        # Write-Host "=== Input ==="
        # Get-Content $inPath
        # Write-Host "=== Your Output ==="
        # Get-Content $outPath
        # Write-Host "=== Expected Output ==="
        # Get-Content $ansPath
        # break
    }
}