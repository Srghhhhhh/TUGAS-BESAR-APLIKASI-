#include "splashscreen.h"
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QPainterPath>

SplashScreen::SplashScreen(const QPixmap &pixmap)
    : QSplashScreen(pixmap, Qt::WindowStaysOnTopHint)
{
    setWindowFlag(Qt::FramelessWindowHint);

    // ── Loading steps  ──────────────────────────────────────
    steps = {
        { 15,  220, "Menyiapkan kanvas..."        },
        { 35,  280, "Memuat palet warna..."        },
        { 52,  200, "Mengasah pensil..."           },
        { 68,  250, "Mengisi ember cat..."         },
        { 80,  180, "Menyalakan efek glow..."      },
        { 92,  200, "Membuka studio gambar..."     },
        { 100, 150, "Siap! Selamat berkreasi..."   },
    };

    // Progress timer – maju pelan-pelan sesuai step
    progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &SplashScreen::onProgressTick);

    // Delay setelah 100% sebelum emit finished
    finishTimer = new QTimer(this);
    finishTimer->setSingleShot(true);
    connect(finishTimer, &QTimer::timeout, this, &SplashScreen::onFinishDelay);

    // Mulai progres seketika
    progressTimer->start(steps[0].delayMs / 15);
}

void SplashScreen::startFinishTimer(int msec)
{
    // Dipanggil dari main() setelah MainWindow selesai di-construct.
    // Jika progress sudah 100%, langsung tunda; jika belum,
    // finishTimer akan ditrigger dari onProgressTick.
    if (progressValue >= 100)
        finishTimer->start(msec);
    // else: finishTimer akan distart setelah 100% di onProgressTick
}

// ─────────────────────────────────────────────────────────────────
void SplashScreen::onProgressTick()
{
    if (currentStep >= steps.size()) {
        progressTimer->stop();
        return;
    }

    int target = steps[currentStep].targetPct;

    if (progressValue < target) {
        progressValue++;
        statusText = steps[currentStep].label;
        repaint();
    } else {
        currentStep++;
        if (currentStep < steps.size()) {
            progressTimer->setInterval(steps[currentStep].delayMs / 15);
        } else {
            progressTimer->stop();
            progressValue = 100;
            repaint();
            // Tunda sedikit sebelum tutup
            finishTimer->start(600);
        }
    }
}

void SplashScreen::onFinishDelay()
{
    emit finished();
}

// ─────────────────────────────────────────────────────────────────
//  Custom render – progress bar + status text di atas gambar
// ─────────────────────────────────────────────────────────────────
void SplashScreen::drawContents(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);

    int W = width();
    int H = height();

    // ── Semi-transparent dark strip di bagian bawah ──
    int stripH = 80;
    int stripY = H - stripH;

    QColor stripBg(30, 20, 40, 210);
    painter->setPen(Qt::NoPen);
    painter->setBrush(stripBg);
    // Rounded hanya sudut bawah
    QPainterPath strip;
    strip.moveTo(0, stripY + 14);
    strip.quadTo(0, stripY, 14, stripY);
    strip.lineTo(W - 14, stripY);
    strip.quadTo(W, stripY, W, stripY + 14);
    strip.lineTo(W, H - 14);
    strip.quadTo(W, H, W - 14, H);
    strip.lineTo(14, H);
    strip.quadTo(0, H, 0, H - 14);
    strip.closeSubpath();
    painter->drawPath(strip);

    // ── Status label ──
    QFont statusFont("Arial", 11, QFont::Normal);
    painter->setFont(statusFont);
    painter->setPen(QColor(220, 210, 255));
    painter->drawText(QRect(16, stripY + 6, W - 32, 22),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      statusText);

    // ── Persentase di kanan ──
    QFont pctFont("Arial", 11, QFont::Bold);
    painter->setFont(pctFont);
    painter->setPen(QColor(180, 160, 255));
    painter->drawText(QRect(0, stripY + 6, W - 12, 22),
                      Qt::AlignRight | Qt::AlignVCenter,
                      QString("%1%").arg(progressValue));

    // ── Progress bar track ──
    int barY  = stripY + 34;
    int barH  = 10;
    int barX  = 14;
    int barW  = W - 28;
    int barR  = barH / 2;

    // Track (gelap)
    painter->setBrush(QColor(60, 50, 80, 180));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(barX, barY, barW, barH, barR, barR);

    // Fill gradient warna-warni
    if (progressValue > 0) {
        int fillW = (barW * progressValue) / 100;
        QLinearGradient grad(barX, barY, barX + barW, barY);
        grad.setColorAt(0.00, QColor(0xFF, 0x6B, 0x6B));  // merah
        grad.setColorAt(0.20, QColor(0xFF, 0x8C, 0x42));  // oranye
        grad.setColorAt(0.40, QColor(0xF9, 0xCA, 0x24));  // kuning
        grad.setColorAt(0.60, QColor(0x4E, 0xCD, 0xC4));  // tosca
        grad.setColorAt(0.80, QColor(0x5B, 0xB8, 0xF5));  // biru
        grad.setColorAt(1.00, QColor(0xA2, 0x9B, 0xFE));  // ungu
        painter->setBrush(grad);
        painter->drawRoundedRect(barX, barY, fillW, barH, barR, barR);

        // Highlight putih atas progress bar (kilap)
        QLinearGradient shine(barX, barY, barX, barY + barH / 2);
        shine.setColorAt(0, QColor(255, 255, 255, 80));
        shine.setColorAt(1, QColor(255, 255, 255, 0));
        painter->setBrush(shine);
        painter->drawRoundedRect(barX, barY, fillW, barH / 2, barR, barR);
    }

    // ── Version text ──
    QFont verFont("Arial", 8);
    painter->setFont(verFont);
    painter->setPen(QColor(140, 130, 160, 180));
    painter->drawText(QRect(0, H - 18, W, 16),
                      Qt::AlignHCenter,
                      "v1.0  ·  Ngebuk Warna Paint App");
}
