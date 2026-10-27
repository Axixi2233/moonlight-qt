# Same-version installer regression

Run on Windows against the actual generated MSI (no installation required):

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/installer/Test-SameVersionUpgrade.ps1 -MsiPath build/build-x64-release/Moonlight.msi
```

The test reads MSI tables using the Windows Installer COM API. It checks that
the current version is included in the upgrade range, old products are removed
inside the transaction before file installation, upgrade identity is retained,
settings registry cleanup excludes upgrades, and newer products are detected.
The original 6.2.0 installer fails the same-version range check.

For deployment verification, ship `scripts/Test-AxixiInstallation.ps1` alongside
`Moonlight-install-manifest.json` in the artifact directory. The JSON contains
`schemaVersion: 1`, `applicationVersion`, and `executableSha256` calculated from
the executable actually included in the MSI. Run the script after installation;
pass `-InstallDirectory` for a non-default installation path. Version equality
alone never counts as success. A hash mismatch or missing executable fails with
a nonzero process exit code.

Actual upgrade/repair and settings-retention acceptance remain separate from
these read-only package and deployment checks.
