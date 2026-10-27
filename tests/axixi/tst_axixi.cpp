#include <QtTest>
#include <QSettings>
#include <QTemporaryDir>
#include <QMetaProperty>

#include "settings/streamingpreferences.h"
#include "cli/commandlineparser.h"
#include "streaming/video/overlaymanager.h"

class OverlaySink : public Overlay::IOverlayRenderer
{
public:
    void notifyOverlayUpdated(Overlay::OverlayType) override {}
};

class AxixiRegressionTest : public QObject
{
    Q_OBJECT
    QTemporaryDir settingsDir;

private slots:
    void initTestCase()
    {
        QVERIFY(settingsDir.isValid());
        QCoreApplication::setOrganizationName("AxixiRegressionTests");
        QCoreApplication::setApplicationName("isolated");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());
    }

    void init()
    {
        QSettings settings;
        settings.clear();
        settings.sync();
        StreamingPreferences::get()->reload();
    }

    void persistedLitePreference()
    {
        auto prefs = StreamingPreferences::get();
        QVERIFY(prefs->showPerformanceOverlayLite);
        QVERIFY(!prefs->showPerformanceOverlay);
        prefs->showPerformanceOverlayLite = false;
        prefs->save();
        QSettings settings;
        QVERIFY(settings.contains("showperfoverlayLite"));
        QCOMPARE(settings.value("showperfoverlayLite").toBool(), false);
        prefs->showPerformanceOverlayLite = true;
        prefs->reload();
        QVERIFY(!prefs->showPerformanceOverlayLite);
        QVERIFY(!prefs->showPerformanceOverlay);
    }

    void qmlLiteChangeNotification()
    {
        auto prefs = StreamingPreferences::get();
        QSignalSpy liteChanged(prefs, &StreamingPreferences::showPerformanceOverlayLiteChanged);
        QSignalSpy visibilityChanged(prefs, &StreamingPreferences::showPerformanceOverlayChanged);
        QVERIFY(prefs->setProperty("showPerformanceOverlayLite", false));
        QCOMPARE(liteChanged.count(), 1);
        QCOMPARE(visibilityChanged.count(), 0);
    }

    void cliOverridesRemainIndependent()
    {
        auto prefs = StreamingPreferences::get();
        StreamCommandLineParser parser;
        parser.parse({"moonlight", "stream", "test-host", "Desktop",
                      "--performance-overlay", "--no-performance-overlay-lite", "--bitrate", "1000000"}, prefs);
        QVERIFY(prefs->showPerformanceOverlay);
        QVERIFY(!prefs->showPerformanceOverlayLite);
        QCOMPARE(prefs->bitrateKbps, 1000000);
        parser.parse({"moonlight", "stream", "test-host", "Desktop",
                      "--no-performance-overlay", "--performance-overlay-lite", "--bitrate", "500"}, prefs);
        QVERIFY(!prefs->showPerformanceOverlay);
        QVERIFY(prefs->showPerformanceOverlayLite);
        QCOMPARE(prefs->bitrateKbps, 500);
    }

    void explicitBitrateSurvivesResolutionChange()
    {
        auto prefs = StreamingPreferences::get();
        prefs->bitrateKbps = 150000;
        prefs->autoAdjustBitrate = false;
        StreamCommandLineParser parser;
        parser.parse({"moonlight", "stream", "test-host", "Desktop", "--resolution", "2560x1440",
                      "--fps", "120", "--bitrate", "150000"}, prefs);
        QCOMPARE(prefs->bitrateKbps, 150000);
        QCOMPARE(prefs->width, 2560);
        QCOMPARE(prefs->fps, 120);
    }

    void overlayKeepsChineseBackgroundAndOwnership()
    {
        OverlaySink sink;
        Overlay::OverlayManager manager;
        manager.setOverlayRenderer(&sink);
        manager.setOverlayStateLite(Overlay::OverlayDebug, true);
        QVERIFY(!manager.isOverlayEnabled(Overlay::OverlayDebug));
        manager.setOverlayState(Overlay::OverlayDebug, true);
        manager.updateOverlayText(Overlay::OverlayDebug, "视频流: 1920x1080 60 FPS\n网络延迟: 2 ms");
        SDL_Surface* surface = manager.getUpdatedOverlaySurface(Overlay::OverlayDebug);
        QVERIFY(surface != nullptr);
        QVERIFY(surface->w > 0 && surface->h > 0);
        // D3D11, libplacebo, VDPAU and Metal upload these pixels as BGRA.
        QCOMPARE(surface->format->format, Uint32(SDL_PIXELFORMAT_ARGB8888));
        SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        QVERIFY(rgba != nullptr);
        const Uint8* pixel = static_cast<const Uint8*>(rgba->pixels);
        QCOMPARE(pixel[0], Uint8(0));
        QCOMPARE(pixel[1], Uint8(0));
        QCOMPARE(pixel[2], Uint8(0));
        QCOMPARE(pixel[3], Uint8(0x50));
        SDL_FreeSurface(rgba);
        SDL_FreeSurface(surface);
        QVERIFY(manager.getUpdatedOverlaySurface(Overlay::OverlayDebug) == nullptr);

        // Updating an unconsumed surface must replace it; disabling clears it.
        for (int i = 0; i < 100; i++) {
            manager.updateOverlayText(Overlay::OverlayDebug, "完整性能信息\n解码延迟: 1.25 ms");
        }
        manager.setOverlayState(Overlay::OverlayDebug, false);
        QVERIFY(manager.getUpdatedOverlaySurface(Overlay::OverlayDebug) == nullptr);
    }

    void overlayTruncationKeepsUtf8Valid()
    {
        Overlay::OverlayManager manager;
        const auto text = QString(600, QChar(0x89e3)).toUtf8();
        manager.updateOverlayText(Overlay::OverlayDebug, text.constData());
        const QByteArray stored(manager.getOverlayText(Overlay::OverlayDebug));
        QVERIFY(stored.size() < manager.getOverlayMaxTextLength());
        QCOMPARE(QString::fromUtf8(stored).toUtf8(), stored);
    }

    void multilineTextRemainsCenteredWithinOutline()
    {
        OverlaySink sink;
        Overlay::OverlayManager manager;
        manager.setOverlayRenderer(&sink);
        manager.setOverlayState(Overlay::OverlayDebug, true);
        manager.setOverlayStateLite(Overlay::OverlayDebug, false);
        manager.updateOverlayText(Overlay::OverlayDebug, "WWWWWWWWWW\nI");
        SDL_Surface* surface = manager.getUpdatedOverlaySurface(Overlay::OverlayDebug);
        QVERIFY(surface != nullptr);
        SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        QVERIFY(rgba != nullptr);
        int left = rgba->w, right = -1;
        for (int y = rgba->h / 2; y < rgba->h; y++) {
            const Uint8* row = static_cast<const Uint8*>(rgba->pixels) + y * rgba->pitch;
            for (int x = 0; x < rgba->w; x++) {
                if (row[x * 4] > 240 && row[x * 4 + 1] > 240 && row[x * 4 + 2] > 240) {
                    left = qMin(left, x);
                    right = qMax(right, x);
                }
            }
        }
        const int width = rgba->w;
        SDL_FreeSurface(rgba);
        SDL_FreeSurface(surface);
        QVERIFY(right >= left);
        QVERIFY2(qAbs(left + right + 1 - width) <= 2,
                 qPrintable(QString("Text bounds [%1,%2] are not centered in width %3").arg(left).arg(right).arg(width)));
    }
};

QTEST_GUILESS_MAIN(AxixiRegressionTest)
#include "tst_axixi.moc"
