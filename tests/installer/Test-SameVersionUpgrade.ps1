param([Parameter(Mandatory=$true)][string]$MsiPath)
$ErrorActionPreference = 'Stop'
$installer = New-Object -ComObject WindowsInstaller.Installer
$database = $installer.OpenDatabase((Resolve-Path -LiteralPath $MsiPath).Path, 0)
function Read-Rows([string]$Query, [int]$Columns) {
    $view = $database.OpenView($Query)
    [void]$view.Execute()
    while ($record = $view.Fetch()) {
        $values = for ($i = 1; $i -le $Columns; $i++) { $record.StringData($i) }
        ,$values
    }
    [void]$view.Close()
}
$properties = @{}
Read-Rows 'SELECT `Property`, `Value` FROM `Property`' 2 | ForEach-Object { $properties[$_[0]] = $_[1] }
$upgrades = @(Read-Rows 'SELECT `VersionMin`, `VersionMax`, `Attributes`, `ActionProperty` FROM `Upgrade`' 4)
$sameVersion = @($upgrades | Where-Object { $_[3] -eq 'WIX_UPGRADE_DETECTED' -and $_[1] -eq $properties.ProductVersion -and ([int]$_[2] -band 512) -ne 0 })
if ($sameVersion.Count -ne 1) { throw 'Same-version product is not included in the MSI upgrade range.' }
$actions = @{}
Read-Rows 'SELECT `Action`, `Sequence` FROM `InstallExecuteSequence`' 2 | ForEach-Object { $actions[$_[0]] = [int]$_[1] }
if (!($actions.InstallInitialize -lt $actions.RemoveExistingProducts -and $actions.RemoveExistingProducts -lt $actions.InstallFiles)) {
    throw 'Old product removal must happen inside the transaction before installing files.'
}
$cleanup = @(Read-Rows "SELECT ``Condition`` FROM ``InstallExecuteSequence`` WHERE ``Action`` = 'DeleteRegistryKey'" 1)
if (($cleanup -join ' ') -notmatch 'NOT UPGRADINGPRODUCTCODE') { throw 'Upgrade must preserve the settings registry.' }
if ($properties.UpgradeCode -ne '{5C09F94E-F809-4C6A-9B7B-597C99F041FE}') { throw 'Upgrade identity changed.' }
if (@($upgrades | Where-Object { $_[3] -eq 'WIX_DOWNGRADE_DETECTED' -and ([int]$_[2] -band 2) -ne 0 }).Count -ne 1) { throw 'Newer product detection is missing.' }
[pscustomobject]@{ ProductCode=$properties.ProductCode; Version=$properties.ProductVersion; SameVersionUpgrade=$true; TransactionalEarlyRemoval=$true; PreservesSettings=$true }
