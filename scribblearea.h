#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QSize>
#include <QStack>
#include <QPen>
#include <QTimer>
#include <QRandomGenerator>

class ScribbleArea : public QWidget
{
    Q_OBJECT

public:
    // Tool enum
    enum Tool {
        Pencil,
        Eraser,
        FloodFill,
        DrawRect,
        DrawEllipse,
        DrawLine
    };

    // Brush style (line pattern)
    enum BrushStyle {
        Solid,
        Dashed,
        Dotted
    };

    // Brush type (painting effect)
    enum BrushType {
        Normal,      // pensil biasa
        Spray,       // semprotan titik-titik
        Marker,      // marker tebal semi-transparan
        Glow,        // cahaya neon berpendar
        Watercolor,  // cat air lembut
        Calligraphy, // pena kaligrafi miring
        Fur,         // bulu/rambut
        Chalk        // kapur tekstur
    };

    explicit ScribbleArea(QWidget *parent = nullptr);

    bool openImage(const QString &fileName);
    bool saveImage(const QString &fileName, const char *fileFormat);

    void setPenColor(const QColor &newColor);
    void setPenWidth(int newWidth);
    void setTool(Tool tool);
    void setBrushStyle(BrushStyle style);
    void setBrushType(BrushType type);

    bool isModified() const { return modified; }
    QColor penColor() const { return myPenColor; }
    int penWidth() const { return myPenWidth; }
    Tool currentTool() const { return myTool; }
    BrushType brushType() const { return myBrushType; }

public slots:
    void clearImage();
    void undo();
    void redo();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // Drawing helpers
    void drawLineTo(QPainter &p, const QPoint &endPoint, const QPoint &startPoint);
    void drawBrushAt(const QPoint &pt, const QPoint &prev);
    void drawSpray(QPainter &p, const QPoint &pt);
    void drawMarker(QPainter &p, const QPoint &pt, const QPoint &prev);
    void drawGlow(QPainter &p, const QPoint &pt, const QPoint &prev);
    void drawWatercolor(QPainter &p, const QPoint &pt, const QPoint &prev);
    void drawCalligraphy(QPainter &p, const QPoint &pt, const QPoint &prev);
    void drawFur(QPainter &p, const QPoint &pt, const QPoint &prev);
    void drawChalk(QPainter &p, const QPoint &pt, const QPoint &prev);

    void resizeImage(QImage *image, const QSize &newSize);
    void floodFill(const QPoint &pos, const QColor &fillColor);
    void pushUndoStack();
    Qt::PenStyle qtPenStyle() const;

    bool modified;
    bool scribbling;
    int myPenWidth;
    QColor myPenColor;
    QImage image;
    QPoint lastPoint;
    QPoint pressPoint;
    Tool myTool;
    BrushStyle myBrushStyle;
    BrushType myBrushType;

    QImage previewImage;

    QStack<QImage> undoStack;
    QStack<QImage> redoStack;
    static const int MAX_UNDO = 20;

    QRandomGenerator rng;
};

#endif // SCRIBBLEAREA_H
