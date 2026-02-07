# ==========================================================
#  Sandbox Evasion (T1497) + Ingress Tool Transfer (T1105)
# ==========================================================
# 
# Install-Module -Name ps2exe -Scope CurrentUser
# Invoke-PS2EXE -InputFile "setup.ps1" -OutputFile "setup.exe" -NoConsole

Add-Type -AssemblyName PresentationFramework

# --- スリープ時間 ---
$s_cmd = "St" + "art-Sl" + "eep"
[System.Windows.MessageBox]::Show("Starting 3-minute sleep...", "Step 1")
& $s_cmd -Seconds 3

# --- EICARテストシグネチャの直接保持 ---
$eicar_signature = 'X5O!P%@AP[4\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*'
[System.Windows.MessageBox]::Show("Checking EICAR signature in memory...", "Step 1: Static Scan Test")

# --- 通信テスト ---
$w_cmd = "Inv" + "oke-Re" + "stMethod"
# $targetUrl = "https://raw.githubusercontent.com/redcanaryco/atomic-red-team/master/atomics/T1059.001/src/test.ps1"
$targetUrl = "https://secure.eicar.org/eicar.com.txt"

try {
    [System.Windows.MessageBox]::Show("Fetching data...", "Step 2: Network Test")
    $data = & $w_cmd -Uri $targetUrl
    
    # 取得データとEICARを結合して保存を試みる（振る舞い検知を誘発）
    $targetFile = "$env:TEMP\detection_test.txt"
    Set-Content -Path $targetFile -Value ($eicar_signature + "`n" + $data)

    [System.Windows.MessageBox]::Show("Warning: Success! (This means detection failed)", "Result")
}
catch {
    $err = $_.Exception.Message
    [System.Windows.MessageBox]::Show("Detected/Blocked by Security!`n`nReason: $err", "Result")
}