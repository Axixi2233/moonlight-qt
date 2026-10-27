# Axixi integration regressions

This small QtTest target compiles the production preferences, CLI parser and
overlay manager. It uses temporary INI settings and an embedded copy of the
actual Chinese font; it never reads or writes the user's Moonlight profile.

From a Windows MSVC x64 developer command prompt at the repository root, with
Qt's `bin` directory on `PATH` and `setup-deps.ps1` already completed:

```bat
set "PATH=%CD%\libs\windows\lib\x64;%PATH%"
mkdir build\tests-axixi
cd build\tests-axixi
qmake ..\..\tests\axixi\axixi.pro "CONFIG+=release"
nmake /nologo
release\axixi-regressions.exe -o results.txt,txt
type results.txt
```

The executable's exit code is the test result. Checks cover legacy preference
persistence/defaults, distinct QML change notifications, positive/negative CLI
overlay switches, bitrate arguments, UTF-8 truncation, Chinese overlay rendering,
background alpha, renderer-compatible ARGB8888 pixels, centered multiline text,
and transferring/discarding generated surfaces.

Hardware decoding, streaming input, installation and non-Windows rendering are
separate manual acceptance checks in `docs/axixi-v6.2.0-upgrade.md`.
