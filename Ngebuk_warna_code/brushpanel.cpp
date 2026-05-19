#include "brushpanel.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QApplication>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolButton>
#include <QRandomGenerator>
#include <cmath>

// ══════════════════════════════════════════════════════════
//  Data brush
// ══════════════════════════════════════════════════════════
struct BrushInfo {
    ScribbleArea::BrushType type;
    QString icon;
    QString label;
    QString desc;
};

static const BrushInfo BRUSHES[] = {
    { ScribbleArea::Normal,      "",  "Normal",     "Pensil solid biasa"           },
    { ScribbleArea::Spray,       "",  "Spray",      "Semprotan titik acak"         },
    { ScribbleArea::Marker,      "",  "Marker",     "Spidol semi-transparan"       },
    { ScribbleArea::Glow,        "",  "Glow",       "Cahaya neon berpendar"        },
    { ScribbleArea::Watercolor,  "",  "Watercolor", "Cat air lembut berbauran"     },
    { ScribbleArea::Calligraphy, "",  "Kaligrafi",  "Pena miring berkarakter"      },
    { ScribbleArea::Fur,         "",  "Fur",        "Bulu halus menyebar"          },
    { ScribbleArea::Chalk,       "",  "Chalk",      "Kapur bertekstur kasar"       },
};
static const int BRUSH_COUNT = static_cast<int>(sizeof(BRUSHES) / sizeof(BRUSHES[0]));

// ══════════════════════════════════════════════════════════
//  BrushButton
// ══════════════════════════════════════════════════════════
BrushButton::BrushButton(ScribbleArea::BrushType type,
                         const QString &label,
                         const QString &desc,
                         QWidget *parent)
    : QAbstractButton(parent),
      myType(type),
      myLabel(label),
      myDesc(desc)
{
    setCheckable(true);
    setFixedSize(108, 88);
    setToolTip(desc);
    setCursor(Qt::PointingHandCursor);
}

void BrushButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    bool sel = isChecked();

    // ── Background card ──
    QColor bg = sel     ? QColor(50, 100, 200)
              : hovered ? QColor(60, 60, 78)
                        : QColor(42, 42, 56);
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 7, 7);

    // ── Border ──
    p.setPen(QPen(sel ? QColor(120, 180, 255) : QColor(75, 75, 100),
                  sel ? 2.0 : 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 7, 7);

    // ── Preview stroke area ──
    QRect previewRect(8, 8, width() - 16, 42);

    // Draw a representative preview based on brush type
    QColor strokeColor = sel ? QColor(200, 230, 255) : QColor(180, 200, 230);

    switch (myType) {
    case ScribbleArea::Normal: {
        p.setPen(QPen(strokeColor, 2.5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(previewRect.left() + 4, previewRect.center().y(),
                   previewRect.right() - 4, previewRect.center().y());
        break;
    }
    case ScribbleArea::Spray: {
        // Scattered dots
        p.setPen(QPen(strokeColor, 1.5));
        QRandomGenerator rng(42);
        for (int i = 0; i < 28; ++i) {
            int x = previewRect.left() + static_cast<int>(rng.generateDouble() * previewRect.width());
            int y = previewRect.top()  + static_cast<int>(rng.generateDouble() * previewRect.height());
            p.drawPoint(x, y);
        }
        break;
    }
    case ScribbleArea::Marker: {
        // Wide semi-transparent flat stroke
        QColor mc = strokeColor; mc.setAlpha(120);
        p.setPen(QPen(mc, 10, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(previewRect.left() + 4, previewRect.center().y(),
                   previewRect.right() - 4, previewRect.center().y());
        QColor mc2 = strokeColor; mc2.setAlpha(200);
        p.setPen(QPen(mc2, 3, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(previewRect.left() + 4, previewRect.center().y(),
                   previewRect.right() - 4, previewRect.center().y());
        break;
    }
    case ScribbleArea::Glow: {
        // Concentric glow layers
        for (int layer = 4; layer >= 1; --layer) {
            QColor gc = QColor(140, 180, 255);
            gc.setAlpha(20 + layer * 15);
            p.setPen(QPen(gc, layer * 3.5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(previewRect.left() + 4, previewRect.center().y(),
                       previewRect.right() - 4, previewRect.center().y());
        }
        p.setPen(QPen(QColor(230, 240, 255), 1.5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(previewRect.left() + 4, previewRect.center().y(),
                   previewRect.right() - 4, previewRect.center().y());
        break;
    }
    case ScribbleArea::Watercolor: {
        // Soft wide + narrow
        QColor wc = strokeColor; wc.setAlpha(50);
        p.setPen(QPen(wc, 12, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(previewRect.left() + 4, previewRect.center().y(),
                   previewRect.right() - 4, previewRect.center().y());
        wc.setAlpha(100);
        p.setPen(QPen(wc, 5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(previewRect.left() + 4, previewRect.center().y(),
                   previewRect.right() - 4, previewRect.center().y());
        break;
    }
    case ScribbleArea::Calligraphy: {
        // Diagonal thick-thin nib
        p.setPen(QPen(strokeColor, 2));
        QPolygon poly;
        int cx = previewRect.center().y();
        poly << QPoint(previewRect.left()+4,  cx+5)
             << QPoint(previewRect.left()+4,  cx-5)
             << QPoint(previewRect.right()-4, cx-10)
             << QPoint(previewRect.right()-4, cx+0);
        QPainterPath path;
        path.addPolygon(poly);
        p.fillPath(path, strokeColor);
        break;
    }
    case ScribbleArea::Fur: {
        // Short radiating lines from center
        int cx = previewRect.center().x();
        int cy = previewRect.center().y();
        p.setPen(QPen(strokeColor, 1));
        QRandomGenerator rng2(7);
        for (int i = 0; i < 18; ++i) {
            double angle = rng2.generateDouble() * 2 * M_PI;
            double len   = 6 + rng2.generateDouble() * 10;
            p.drawLine(cx, cy,
                       cx + static_cast<int>(len * std::cos(angle)),
                       cy + static_cast<int>(len * std::sin(angle)));
        }
        break;
    }
    case ScribbleArea::Chalk: {
        // Rough textured dots along line
        p.setPen(QPen(strokeColor, 1));
        QRandomGenerator rng3(13);
        int y0 = previewRect.center().y();
        for (int x = previewRect.left() + 4; x < previewRect.right() - 4; x += 2) {
            for (int t = 0; t < 3; ++t) {
                int ox = static_cast<int>((rng3.generateDouble()-0.5)*6);
                int oy = static_cast<int>((rng3.generateDouble()-0.5)*6);
                QColor cc = strokeColor;
                cc.setAlpha(80 + static_cast<int>(rng3.generateDouble()*120));
                p.setPen(QPen(cc, 1));
                p.drawPoint(x + ox, y0 + oy);
            }
        }
        break;
    }
    }

    // ── Label ──
    QFont labelFont = p.font();
    labelFont.setPointSize(8);
    labelFont.setBold(sel);
    p.setFont(labelFont);
    p.setPen(sel ? Qt::white : QColor(210, 215, 230));
    p.drawText(QRect(2, 52, width() - 4, 16),
               Qt::AlignHCenter | Qt::AlignVCenter, myLabel);

    // ── Desc ──
    QFont descFont = p.font();
    descFont.setPointSize(6);
    descFont.setBold(false);
    p.setFont(descFont);
    p.setPen(QColor(130, 135, 160));
    p.drawText(QRect(2, 67, width() - 4, 16),
               Qt::AlignHCenter | Qt::AlignVCenter, myDesc);
}

void BrushButton::enterEvent(QEnterEvent *) { hovered = true;  update(); }
void BrushButton::leaveEvent(QEvent *)      { hovered = false; update(); }

// ══════════════════════════════════════════════════════════
//  BrushPanel
// ══════════════════════════════════════════════════════════
BrushPanel::BrushPanel(QWidget *parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);

    setStyleSheet(
        "BrushPanel {"
        "  background: #1e1e2e;"
        "  border: 1.5px solid #3c3c58;"
        "  border-radius: 12px;"
        "}"
    );

    btnGroup = new QButtonGroup(this);
    btnGroup->setExclusive(true);

    // ── Judul ──
    auto *titleLabel = new QLabel("Pilih Brush", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "color:#cdd6f4; font-size:13px; font-weight:bold;"
        "padding:8px 0 4px 0; background:transparent;");

    // ── Grid 4 kolom ──
    auto *grid = new QGridLayout;
    grid->setSpacing(6);
    grid->setContentsMargins(10, 4, 10, 10);

    for (int i = 0; i < BRUSH_COUNT; ++i) {
        const BrushInfo &bi = BRUSHES[i];
        auto *btn = new BrushButton(bi.type, bi.label, bi.desc, this);
        btnGroup->addButton(btn, i);
        grid->addWidget(btn, i / 4, i % 4);
    }

    // Default: Normal terpilih
    if (auto *b = btnGroup->button(0)) b->setChecked(true);

    connect(btnGroup, &QButtonGroup::idClicked, this, [this](int id) {
        if (id >= 0 && id < BRUSH_COUNT)
            emit brushSelected(BRUSHES[id].type);
        hide();
    });

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(grid);
    setLayout(mainLayout);

    // Tutup saat klik di luar panel
    qApp->installEventFilter(this);
}

void BrushPanel::setCurrentBrush(ScribbleArea::BrushType t)
{
    for (int i = 0; i < BRUSH_COUNT; ++i) {
        if (BRUSHES[i].type == t) {
            if (auto *btn = btnGroup->button(i))
                btn->setChecked(true);
            return;
        }
    }
}

void BrushPanel::setPreviewColor(const QColor &c)
{
    previewColor = c;
    update();
}

bool BrushPanel::eventFilter(QObject *, QEvent *event)
{
    if (!isVisible()) return false;
    if (event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (!geometry().contains(me->globalPosition().toPoint()))
            hide();
    }
    return false;
}
