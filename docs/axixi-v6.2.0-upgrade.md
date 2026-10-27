# Axixi v6.2.0 upgrade

## Accepted scope

- Preserve master `18c9f089fdb020dadea6694ca6b7d242deb4320d`, including the March 19 Linux overlay changes.
- Merge upstream v6.2.0 `de2467e433821664cdd2224aad8c89a625be1ad9` on `sync/upstream-v6.2.0`; do not rewrite history, push, or publish.
- Build Windows x64 locally and deliver an unsigned portable ZIP and installer, with checksums.
- Preserve pairing/settings identity, 1000 Mbps unlocked bitrate, Chinese centered full/lite statistics, OPPOSans-H, purple SVG, keyboard and Guide-button shortcuts.
- Retain upstream rendering/input fixes, dependency migration, and non-Windows sources. User performs hardware streaming and install/upgrade acceptance.

## Implementation tasks

1. Merge the fixed tag; reconcile every Axixi delta against the common ancestor `fabb4fdadc302493d02bb23485dbc15d7988834b`.
2. Preserve configuration/CLI contracts, overlay appearance and rendering ownership, and input combinations on the new upstream implementation.
3. Adapt WiX 7 and CI to x64-only Windows packaging. Retain installer upgrade identity and current VC runtime metadata.
4. Provision tag-pinned dependencies and Qt/MSVC tools; compile, package, and exercise meaningful automated checks.
5. Review the complete integration, fix regressions, and document evidence and manual acceptance steps.

## Compatibility contracts

- `showperfoverlayLite` remains case-sensitive with default `true`; `showPerformanceOverlayLite` and both CLI toggle forms remain available.
- Overlay visibility and full/lite presentation are independent. Ctrl+Alt+Shift+A and Guide+B switch presentation; Guide+Y toggles visibility; Guide+A switches mouse mode; Guide+X quits subject to `NO_GAMEPAD_QUIT`.
- Keep original shortcuts, upstream Ctrl+Alt+Shift+K, and the original handling of locally consumed controller-button events.
- Preserve unlocked/locked GUI bitrate maxima of 1000000/150000 Kbps; keep the CLI warning range 500–1000000 Kbps and upstream manually selected bitrate retention.
- `app/version.txt` is 6.2.0. Delivery names are `Moonlight-Axixi-6.2.0-x64.zip` and `Moonlight-Axixi-6.2.0-x64-Setup.exe`.

## Validation

- Git ancestry, conflict-marker and whitespace checks; compare the complete fork delta against v6.2.0.
- Focused runtime regressions for settings, CLI, UTF-8 overlay text and surface ownership; Windows x64 Release build, ZIP/EXE packaging, startup and dependency checks.
- User acceptance: retained pairing/settings; full/lite overlay and keyboard/gamepad controls; resizing/DPI/reconnect; supported codecs, HDR and VRR; clean install, old Axixi upgrade, uninstall.
- Portable upgrades use a new directory and migrate settings, avoiding leftover Qt TLS plugins. Non-Windows hardware results are not claimed without testing.

## Implementation record

- Started from a clean checkout. Created the approved branch in place; master remains unchanged.
- Ruling: use branch isolation in the existing checkout, matching the accepted plan; no additional worktree is needed. A future checkout must preserve any uncommitted integration work.
- Baseline build unavailable before dependency setup: submodules were uninitialized and Qt was not installed. No existing automated test suite was found.
- Merge initially had nine content conflicts; both conflicting and automatically merged custom files were reviewed.
- All content conflicts reconciled. Retained upstream microsecond statistics, optional bitrate display, UTF-8 copy and surface exchange/free; added Axixi presentation on top.
- Added centered placement to libplacebo because v6.2.0 changes the default macOS renderer away from Metal. Corrected VAAPI positioning using the current window dimensions.
- Qt 6.12.0 installed with upstream-pinned aqt commit `073e34d7c2ab4ae6961ed7cca690b3abd5ba5a7e`; the released aqt 3.3.0 could not parse its repository layout. Dependencies and recursive submodules are now initialized at the v6.2.0 versions.
- Ruling: use the complete, installed VS 2022 17.14 / MSVC 14.44 toolchain with Qt's MSVC2022 binaries. The VS 18 Insiders installation lacks the C++ headers/setup scripts. Upstream permits older toolchains without official support; full compile and runtime checks determine suitability. If this fails, provision a complete supported toolchain instead of changing the source baseline.
- Focused regressions reproduced two failures before correction: the lite QML property notified the visibility signal, and the merged outlined overlay lacked the translucent background. Both were corrected; test rebuild/execution returned success. The configuration/CLI contracts and UTF-8 truncation also pass.
- Ruling: retain upstream CLI semantics when resolution/FPS changes without an explicit bitrate. Automatic preservation of manually selected bitrate is a GUI behavior; the CLI honors an explicit `--bitrate` value. Broadening CLI semantics would exceed this integration.
- Independent review found a renderer pixel-format contract violation and a centered multiline outline offset. New runtime regressions reproduced both (wrong format; text bounds 615–617 in width 1224). Corrected to ARGB8888 and centered foreground placement.
- Ruling: treat the newly introduced outline displacement as part of the required Axixi centering regression fix, alongside the pixel-format fix, rather than deferring a known visual regression. Both are verified by the actual SDL_ttf renderer.
- Ruling: preserve the original fork's Guide-combination handling, including its existing later-axis-event behavior; redesigning controller state consumption is outside this upstream sync. Hardware shortcut behavior remains a user acceptance item.

## Results

- All merge conflicts are resolved. `git diff --cached --check v6.2.0` passes for the Axixi integration delta. Checking the entire merge against the old fork also reports whitespace already present in six unchanged upstream files; those files remain byte-for-byte identical to v6.2.0, including the embedded patch file. Ruling: preserve upstream content rather than add unrelated formatting edits.
- Windows x64 Release compilation, deployment, portable ZIP creation, MSI build, and WiX bundle build succeeded. Final packages were rebuilt after the review fixes.
- QtTest: 9 passed, 0 failed, 0 skipped (7 functional checks plus initialization/cleanup). The tests compile the production preferences, CLI and SDL_ttf overlay renderer. Evidence: `build/regressions-current.txt` and `build/regressions-green-build.log`.
- Clean portable deployment returned `Moonlight 6.2.0` and exit code 0 for `--version`. A hidden GUI startup stayed responsive; logs recorded successful UI initialization and a hardware-decoder capability probe. This does not establish visible layout or actual streaming correctness. The test process was closed after inspection.
- Portable ZIP contains `portable.dat` and Qt Schannel/certificate TLS plugins; the obsolete `qopensslbackend.dll` is absent.
- Extracted installer manifest contains only `Moonlight_x64` 6.2.0.0 and `VC_redist.x64.exe` 14.51.36247.0. x64 launch conditions and original product/bundle upgrade identities are retained. The installer was inspected, not installed.
- The application compiler reports inherited SDL/Qt macro redefinition and FFmpeg deprecation warnings. MSI/bundle builds report zero warnings/errors. Startup logs include the upstream QML multi-binding Shortcut warnings. No warning-free application build is claimed.
- Toolchain: Qt 6.12.0 MSVC2022 x64, Visual Studio 2022 17.14.23, MSVC 14.44.35207, Windows SDK 10.0.26100, WiX 7.0.0, upstream Windows dependency release v19.
- Deliverables and SHA-256 values are in `build/artifacts/`. The Windows binaries are unsigned. See [Chinese delivery and acceptance notes](axixi-v6.2.0-release-notes.zh-CN.md).
- Actual streaming, controller input, HDR/VRR, clean installation, upgrade/uninstall, and non-Windows execution remain manual acceptance items. These results are not implied by a successful build or startup probe.
- Follow-up: an actual installation over upstream 6.2.0 retained its equal-version executable. The installer now includes equal versions in its major-upgrade range; packaging and deployment hash checks were added. See [same-version installer fix](axixi-same-version-install-fix.md). The new installer reuses the validated application payload; user install/streaming acceptance remains pending.
