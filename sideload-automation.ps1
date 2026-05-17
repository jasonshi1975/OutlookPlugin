# Outlook Web Add-in Automation Sideload
$manifestUrl = 'https://jasonshi1975.github.io/OutlookPlugin/manifest.xml'

try {
    # 获取Outlook Application对象
    $outlook = [Runtime.Interopservices.Marshal]::GetActiveObject('Outlook.Application')
    if (-not $outlook) {
        $outlook = New-Object -ComObject Outlook.Application
    }

    # 获取ActiveExplorer
    $explorer = $outlook.ActiveExplorer()
    if ($explorer) {
        Write-Output 'Outlook Explorer obtained'
        
        # 尝试打开加载项页面
        # CommandBars ID for Get Add-ins = 3923
        $commandBars = $explorer.CommandBars
        $addinsBtn = $commandBars.FindControl(Type:=1, Id:=3923)
        if ($addinsBtn) {
            $addinsBtn.Execute()
            Write-Output 'Get Add-ins dialog opened'
        } else {
            Write-Output 'Get Add-ins button not found'
        }
    }
} catch {
    Write-Output "Error: $($_.Exception.Message)"
}
