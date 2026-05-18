#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
#include <QPixmap>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QPainter>
#include <QFont>

// ─────────────────────────────────────────────────────────────────
//  SplashScreen
//  Tampil di atas semua window, menampilkan SVG + progress bar
//  animasi, lalu menutup sendiri setelah aplikasi siap.
// ─────────────────────────────────────────────────────────────────
class SplashScreen : public QSplashScreen
{
    Q_OBJECT

public:
    explicit SplashScreen(const QPixmap &pixmap);

    // Panggil ini setelah MainWindow siap untuk mulai hitung mundur.
    void startFinishTimer(int msec = 800);

signals:
    void finished();

protected:
    void drawContents(QPainter *painter) override;
    void mousePressEvent(QMouseEvent *) override {}  // blokir klik tutup paksa

private slots:
    void onProgressTick();
    void onFinishDelay();

private:
    QTimer      *progressTimer;
    QTimer      *finishTimer;
    int          progressValue = 0;

    struct LoadStep {
        int  targetPct;
        int  delayMs;
        QString label;
    };
    QList<LoadStep> steps;
    int currentStep = 0;

    QString statusText = "Memuat...";
};

#endif // SPLASHSCREEN_H
