#ifndef BRUSHPANEL_H
#define BRUSHPANEL_H

#include <QFrame>
#include <QAbstractButton>   // ← HARUS include penuh, bukan forward-declare
#include <QButtonGroup>
#include <QPaintEvent>
#include <QEnterEvent>
#include "scribblearea.h"

// ─────────────────────────────────────────────────────────────────
//  BrushButton  –  satu kartu brush di dalam panel
// ─────────────────────────────────────────────────────────────────
class BrushButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit BrushButton(ScribbleArea::BrushType type,
                         const QString &label,
                         const QString &desc,
                         QWidget *parent = nullptr);

    ScribbleArea::BrushType brushType() const { return myType; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event)      override;

private:
    ScribbleArea::BrushType myType;
    QString myLabel;
    QString myDesc;
    bool    hovered = false;
};

// ─────────────────────────────────────────────────────────────────
//  BrushPanel  –  popup grid pilih brush
// ─────────────────────────────────────────────────────────────────
class BrushPanel : public QFrame
{
    Q_OBJECT

public:
    explicit BrushPanel(QWidget *parent = nullptr);

    void setCurrentBrush(ScribbleArea::BrushType t);
    void setPreviewColor(const QColor &c);

signals:
    void brushSelected(ScribbleArea::BrushType type);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QButtonGroup *btnGroup;
    QColor        previewColor = Qt::black;
};

#endif // BRUSHPANEL_H
