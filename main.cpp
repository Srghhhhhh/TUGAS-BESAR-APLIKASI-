#include <QApplication>
#include <QPixmap>
#include <QIcon>
#include <QSvgRenderer>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QLocale>
#include <QTranslator>

#include "mainwindow.h"
#include "splashscreen.h"

// ─────────────────────────────────────────────────────────────────
//  Helper: render SVG ke QPixmap dengan ukuran tertentu
// ─────────────────────────────────────────────────────────────────
static QPixmap svgToPixmap(const QString &svgPath, const QSize &size)
{
    QPixmap pix(size);
    pix.fill(Qt::transparent);
    QSvgRenderer renderer(svgPath);
    if (!renderer.isValid())
        return pix;
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    renderer.render(&p);
    return pix;
}

// ─────────────────────────────────────────────────────────────────
//  Helper: buat QIcon multi-resolusi dari SVG
// ─────────────────────────────────────────────────────────────────
static QIcon svgToIcon(const QString &svgPath)
{
    QIcon icon;
    for (int sz : {16, 24, 32, 48, 64, 128, 256}) {
        QPixmap pix = svgToPixmap(svgPath, QSize(sz, sz));
        if (!pix.isNull())
            icon.addPixmap(pix);
    }
    return icon;
}

// ─────────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
    // High-DPI support (Qt 6 otomatis, Qt 5 perlu ini)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    app.setApplicationName("Ngebuk Warna");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("NgebukStudio");

    // ── Translator ──────────────────────────────────────────
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        if (translator.load(":/i18n/PAINT_" + QLocale(locale).name())) {
            app.installTranslator(&translator);
            break;
        }
    }

    // ── App Icon (window titlebar + taskbar + exe) ───────────
    const QString svgFile = ":/images/splash.svg";
    QIcon appIcon = svgToIcon(svgFile);
    if (!appIcon.isNull())
        app.setWindowIcon(appIcon);

    // ── Splash Screen ────────────────────────────────────────
    // Ukuran splash: 580×620 sesuai viewBox SVG, di-scale kalau layar kecil
    QScreen *primaryScreen = app.primaryScreen();
    QSize screenSize = primaryScreen->availableSize();

    // Splash max 70% tinggi layar, jaga aspect ratio
    int splashH = qMin(620, static_cast<int>(screenSize.height() * 0.70));
    int splashW = static_cast<int>(splashH * 580.0 / 620.0);
    QSize splashSize(splashW, splashH);

    QPixmap splashPix = svgToPixmap(svgFile, splashSize);

    // Rounded corners pada pixmap
    {
        QPixmap rounded(splashSize);
        rounded.fill(Qt::transparent);
        QPainter rp(&rounded);
        rp.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rounded.rect(), 28, 28);
        rp.setClipPath(path);
        rp.drawPixmap(0, 0, splashPix);
        splashPix = rounded;
    }

    SplashScreen *splash = new SplashScreen(splashPix);

    // Posisikan splash di tengah layar
    splash->move(primaryScreen->geometry().center() -
                 QPoint(splashW / 2, splashH / 2));
    splash->show();
    app.processEvents();   // pastikan splash tergambar sebelum loading

    // ── Construct MainWindow ─────────────────────────────────
    MainWindow *window = new MainWindow;
    window->setWindowIcon(appIcon);   // icon juga di window title

    // ── Tutup splash, tampilkan main window ──────────────────
    QObject::connect(splash, &SplashScreen::finished, [=]() {
        splash->finish(window);   // fade-out dan hapus splash
        window->show();
        splash->deleteLater();
    });

    // Beri tahu splash bahwa app sudah siap (tunda min 400ms
    // supaya progress bar sempat kelihatan jika loading sangat cepat)
    splash->startFinishTimer(400);

    return app.exec();
}
