# Axixi 同版本覆盖安装修复

## 目标与实现

在 `sync/upstream-v6.2.0` 分支上修复安装升级行为，版本仍为 6.2.0.0。官方版和 Axixi 版使用相同版本号时，新安装器应替换旧主程序，并保留配对、设置及快捷方式。默认安装路径不变；使用自定义路径时，请在安装器选项中选择原目录。

旧包的实际安装日志显示 `Moonlight.exe; Won't Overwrite; Existing file is of an equal version`。本机安装文件仍有上游签名，哈希不同于 Axixi 构建。旧 MSI 的 Upgrade 表未将当前版本纳入升级范围。

启用 `MajorUpgrade AllowSameVersionUpgrades="yes"`，继续使用 `afterInstallInitialize`：在 MSI 事务内、安装文件前移除旧产品，使旧产品以升级方式卸载。原有 `NOT UPGRADINGPRODUCTCODE` 条件保护配置注册表。新 ProductCode 与旧包不同，UpgradeCode 保持不变。

同一安装器再次运行时使用现有 Burn 修复流程。不以 `MsiProperty` 设置由 Burn 控制的 `REINSTALLMODE`。不改变应用代码、版本号、统计信息实现或便携 ZIP。

Windows Installer 的产品版本比较使用前三段。因此“同版本”按 6.2.0 比较；6.2.0.x 之间允许替换，较高主次补丁版本仍阻止降级。

## 验证记录

- 旧 MSI 检查已失败：`Same-version product is not included in the MSI upgrade range.`
- 后续检查包括新 MSI 的版本上界包含标志、事务内早期移除、配置保护、降级检测，以及新旧 ProductCode 差异。
- 新 MSI 的上述检查已通过；新 ProductCode 为 `{E8F2D8B8-3E88-4281-B0B5-7FA339C378FF}`，旧包为 `{A01D41B0-18C0-475A-90DC-0D413A9FDE5A}`。MSI 与 Setup 构建均成功，零警告／错误。
- 提取新安装器内 MSI 和主程序，确认主程序哈希与已验证的 Axixi 构建一致。
- 包内 273 个文件的文件标识、名称和内容哈希均与已验证的旧 Axixi 包一致；安装规则发生变化，应用及依赖内容未变。
- `scripts/Test-AxixiInstallation.ps1` 使用交付清单中的 SHA-256 检查安装结果，不仅比较版本号。它不启动程序，也不读取或修改用户配置。
- Windows PowerShell 5.1 下验证通过：旧 MSI 被拒绝、新 MSI 通过；已安装的官方同版本主程序被拒绝，Axixi 程序通过，缺失主程序被拒绝。检查脚本为 `tests/installer/Test-SameVersionUpgrade.ps1`，日志位于 `build/same-version-*-check.log`、`build/installed-original-check.log`、`build/portable-axixi-check.log`。
- 实际覆盖安装、修复和串流验收按既定约定由用户执行；构建和包检查不能替代这些结果。

## 安装后检查

退出 Moonlight，直接运行更新后的 `Moonlight-Axixi-6.2.0-x64-Setup.exe`，无需先卸载旧版。安装完成后，在交付目录执行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Test-AxixiInstallation.ps1
```

如果使用自定义安装路径，加上 `-InstallDirectory '你的安装目录'`。输出 `PASS` 才表示主程序与本次 Axixi 构建一致；错误会返回非零退出码。新安装器、清单和脚本应保存在一起。

需要验收官方 6.2.0 → 修复版、此前 Axixi 包 → 修复版，以及同一修复包的重复运行／修复。每种场景都应核对主程序哈希、配对与设置保留、快捷方式及中文统计样式。

## 执行记录

- 基线：`8052fd698d6324c469d6df0c65bd1f1c796dc058`；修改前工作区干净。
- Pre-flight: no shared interfaces beyond the generated MSI and its bundled executable.
- 按已批准方案复用现有 Axixi 主程序，只重新生成 MSI 与 Setup。重新打包时使用旧 MSI 提取的依赖目录，避免将便携模式标记和便携 VC++ 运行库混入安装包。
- Final review: 独立只读审查无 Critical / Important 问题，复核新旧 MSI 的升级范围、配置保护、全部 273 个有效载荷文件与清单哈希。
- Ruling: 自定义安装目录自动发现保持原行为；修正文档中无条件保留路径的表述，要求在安装器中选择原目录。若未选择，安装器会使用默认目录。本次不扩展目录迁移功能。
- Ruling: 覆盖安装、重复修复、失败回滚和串流实测仍按约定留给用户；静态 MSI 检查不能证明这些运行结果。
- Ruling: 旧 MSI 的 LocalAppData 缓存清理保持原行为；配对及偏好存于受升级保护的 QSettings 注册表，缓存可能需要重建。
- Ruling: 不重审或更改上游同步和应用行为；本次以全部有效载荷哈希不变为依据沿用先前应用验证，先前实机验收限制继续适用。
- Ruling: 安装后脚本仅证明主程序内容正确，不证明 MSI 注册状态或实际运行成功；安装与串流验收仍需独立完成。

参考：[WiX MajorUpgrade](https://docs.firegiant.com/wix/schema/wxs/majorupgrade/)、[Burn MSI 属性限制](https://docs.firegiant.com/wix/schema/wxs/msiproperty/)。
