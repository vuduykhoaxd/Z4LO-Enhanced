# Read wide extract files in 2000-byte chunks, search for target patterns
$files = @(
    "wide_extract_1299091.txt",
    "wide_extract_1299128.txt",
    "wide_extract_1631530.txt",
    "wide_extract_2018836.txt",
    "wide_extract_2192385.txt",
    "wide_extract_2378709.txt",
    "wide_extract_2680965.txt",
    "wide_extract_2703115.txt"
)

$patterns = @("ipcMain", "handle", "on(", "zbackup", "z-hide-app-before-quit")

foreach ($f in $files) {
    $path = "C:\Users\xd\Documents\GitHub\Z4LOEnhanced\re\" + $f
    if (-not (Test-Path $path)) {
        Write-Host "MISSING: $f"
        continue
    }
    $bytes = [System.IO.File]::ReadAllBytes($path)
    $total = $bytes.Length
    Write-Host "=== $f ($total bytes) ==="
    
    for ($offset = 0; $offset -lt $total; $offset += 2000) {
        $len = [Math]::Min(2000, $total - $offset)
        $chunk = [System.Text.Encoding]::UTF8.GetString($bytes, $offset, $len)
        $found = $patterns | Where-Object { $chunk -match $_ }
        if ($found.Count -gt 0) {
            Write-Host "  [$offset..$($offset+$len-1)] MATCH: $($found -join ', ')"
            # Write matching chunk to file for review
            $outFile = "C:\Users\xd\Documents\GitHub\Z4LOEnhanced\re\match_$f.`$offset.txt"
            [System.IO.File]::WriteAllText($outFile, $chunk)
        } else {
            Write-Host "  [$offset..$($offset+$len-1)] no match"
        }
    }
}