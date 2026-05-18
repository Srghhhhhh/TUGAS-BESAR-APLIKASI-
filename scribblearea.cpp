#include <QtWidgets>
#include <cmath>
#include "scribblearea.h"

ScribbleArea::ScribbleArea(QWidget *parent)
    : QWidget(parent),
      modified(false),
      scribbling(false),
      myPenWidth(8),
      myPenColor(Qt::black),
      myTool(Pencil),
      myBrushStyle(Solid),
      myBrushType(Normal),
      rng(QRandomGenerator::global()->generate())
{
    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);
}

// ─────────────────────────────────────────────
//  File I/O
// ─────────────────────────────────────────────
bool ScribbleArea::openImage(const QString &fileName)
{
    QImage loadedImage;
    if (!loadedImage.load(fileName)) return false;
    QSize newSize = loadedImage.size().expandedTo(size());
    resizeImage(&loadedImage, newSize);
    image = loadedImage;
    previewImage = image;
    modified = false;
    update();
    return true;
}

bool ScribbleArea::saveImage(const QString &fileName, const char *fileFormat)
{
    QImage visibleImage = image;
    resizeImage(&visibleImage, size());
    if (visibleImage.save(fileName, fileFormat)) {
        modified = false;
        return true;
    }
    return false;
}

// ─────────────────────────────────────────────
//  Setters
// ─────────────────────────────────────────────
void ScribbleArea::setPenColor(const QColor &c)  { myPenColor = c; }
void ScribbleArea::setPenWidth(int w)             { myPenWidth = w; }
void ScribbleArea::setBrushStyle(BrushStyle s)    { myBrushStyle = s; }
void ScribbleArea::setBrushType(BrushType t)      { myBrushType = t; }

void ScribbleArea::setTool(Tool tool)
{
    myTool = tool;
    setCursor(tool == FloodFill ? Qt::PointingHandCursor : Qt::CrossCursor);
}

// ─────────────────────────────────────────────
//  Undo / Redo
// ─────────────────────────────────────────────
void ScribbleArea::pushUndoStack()
{
    if (undoStack.size() >= MAX_UNDO) undoStack.removeFirst();
    undoStack.push(image);
    redoStack.clear();
}

void ScribbleArea::undo()
{
    if (undoStack.isEmpty()) return;
    redoStack.push(image);
    image = undoStack.pop();
    previewImage = image;
    modified = true;
    update();
}

void ScribbleArea::redo()
{
    if (redoStack.isEmpty()) return;
    undoStack.push(image);
    image = redoStack.pop();
    previewImage = image;
    modified = true;
    update();
}

// ─────────────────────────────────────────────
//  Clear
// ─────────────────────────────────────────────
void ScribbleArea::clearImage()
{
    pushUndoStack();
    image.fill(qRgb(255, 255, 255));
    previewImage = image;
    modified = true;
    update();
}

// ─────────────────────────────────────────────
//  Mouse Events
// ─────────────────────────────────────────────
void ScribbleArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    pressPoint = event->pos();
    lastPoint  = event->pos();

    if (myTool == FloodFill) {
        pushUndoStack();
        floodFill(event->pos(), myPenColor);
        modified = true;
        update();
        return;
    }

    scribbling = true;
    pushUndoStack();
    previewImage = image;
}

void ScribbleArea::mouseMoveEvent(QMouseEvent *event)
{
    if (!scribbling) return;

    if (myTool == Pencil || myTool == Eraser) {
        drawBrushAt(event->pos(), lastPoint);
        lastPoint = event->pos();
    } else {
        QImage tmp = previewImage;
        QPainter p(&tmp);
        QPen pen(myPenColor, myPenWidth, qtPenStyle(), Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setRenderHint(QPainter::Antialiasing);
        QRect r = QRect(pressPoint, event->pos()).normalized();
        switch (myTool) {
        case DrawRect:    p.drawRect(r);                        break;
        case DrawEllipse: p.drawEllipse(r);                    break;
        case DrawLine:    p.drawLine(pressPoint, event->pos()); break;
        default: break;
        }
        image = tmp;
        update();
    }
}

void ScribbleArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !scribbling) return;

    if (myTool == Pencil || myTool == Eraser) {
        drawBrushAt(event->pos(), lastPoint);
    } else {
        QPainter p(&previewImage);
        QPen pen(myPenColor, myPenWidth, qtPenStyle(), Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setRenderHint(QPainter::Antialiasing);
        QRect r = QRect(pressPoint, event->pos()).normalized();
        switch (myTool) {
        case DrawRect:    p.drawRect(r);                        break;
        case DrawEllipse: p.drawEllipse(r);                    break;
        case DrawLine:    p.drawLine(pressPoint, event->pos()); break;
        default: break;
        }
        image = previewImage;
    }

    scribbling = false;
    modified = true;
    update();
}

void ScribbleArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect dirty = event->rect();
    painter.drawImage(dirty, image, dirty);
}

void ScribbleArea::resizeEvent(QResizeEvent *event)
{
    if (width() > image.width() || height() > image.height()) {
        resizeImage(&image,
                    QSize(qMax(width() + 128, image.width()),
                          qMax(height() + 128, image.height())));
        previewImage = image;
        update();
    }
    QWidget::resizeEvent(event);
}

// ─────────────────────────────────────────────
//  Brush Dispatcher
// ─────────────────────────────────────────────
void ScribbleArea::drawBrushAt(const QPoint &pt, const QPoint &prev)
{
    if (myTool == Eraser) {
        QPainter p(&image);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::white, myPenWidth * 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawLine(prev, pt);
        int r = myPenWidth * 3 / 2 + 2;
        update(QRect(prev, pt).normalized().adjusted(-r,-r,r,r));
        return;
    }

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);

    switch (myBrushType) {
    case Normal:      drawLineTo(p, pt, prev);     break;
    case Spray:       drawSpray(p, pt);            break;
    case Marker:      drawMarker(p, pt, prev);     break;
    case Glow:        drawGlow(p, pt, prev);       break;
    case Watercolor:  drawWatercolor(p, pt, prev); break;
    case Calligraphy: drawCalligraphy(p, pt, prev);break;
    case Fur:         drawFur(p, pt, prev);        break;
    case Chalk:       drawChalk(p, pt, prev);      break;
    }

    int r = myPenWidth + 20;
    update(QRect(prev, pt).normalized().adjusted(-r,-r,r,r));
    modified = true;
}

// ─────────────────────────────────────────────
//  Normal
// ─────────────────────────────────────────────
void ScribbleArea::drawLineTo(QPainter &p, const QPoint &endPoint, const QPoint &startPoint)
{
    p.setPen(QPen(myPenColor, myPenWidth, qtPenStyle(), Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(startPoint, endPoint);
}

// ─────────────────────────────────────────────
//  Spray
// ─────────────────────────────────────────────
void ScribbleArea::drawSpray(QPainter &p, const QPoint &pt)
{
    int radius = myPenWidth * 2;
    int dots   = myPenWidth * 3 + 20;
    QColor c   = myPenColor;
    c.setAlpha(180);
    p.setPen(QPen(c, 1));
    for (int i = 0; i < dots; ++i) {
        double angle = rng.generateDouble() * 2 * M_PI;
        double dist  = rng.generateDouble() * radius;
        int dx = static_cast<int>(dist * std::cos(angle));
        int dy = static_cast<int>(dist * std::sin(angle));
        p.drawPoint(pt + QPoint(dx, dy));
    }
}

// ─────────────────────────────────────────────
//  Marker
// ─────────────────────────────────────────────
void ScribbleArea::drawMarker(QPainter &p, const QPoint &pt, const QPoint &prev)
{
    QColor c = myPenColor;
    c.setAlpha(80);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.setPen(QPen(c, myPenWidth * 2.5, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
    p.drawLine(prev, pt);
}

// ─────────────────────────────────────────────
//  Glow
// ─────────────────────────────────────────────
void ScribbleArea::drawGlow(QPainter &p, const QPoint &pt, const QPoint &prev)
{
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QColor c = myPenColor;
    int layers = 5;
    for (int i = layers; i >= 1; --i) {
        c.setAlpha(15 + i * 8);
        p.setPen(QPen(c, myPenWidth * (i + 1) * 0.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawLine(prev, pt);
    }
    c.setAlpha(230);
    p.setPen(QPen(c, myPenWidth * 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(prev, pt);
    QColor white(255, 255, 255, 160);
    p.setPen(QPen(white, myPenWidth * 0.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(prev, pt);
}

// ─────────────────────────────────────────────
//  Watercolor
// ─────────────────────────────────────────────
void ScribbleArea::drawWatercolor(QPainter &p, const QPoint &pt, const QPoint &prev)
{
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QColor c = myPenColor;
    c.setAlpha(30);
    p.setPen(QPen(c, myPenWidth * 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(prev, pt);
    c.setAlpha(60);
    p.setPen(QPen(c, myPenWidth * 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(prev, pt);
    c.setAlpha(20);
    p.setPen(QPen(c, 2));
    for (int i = 0; i < 6; ++i) {
        int dx = static_cast<int>((rng.generateDouble() - 0.5) * myPenWidth * 2);
        int dy = static_cast<int>((rng.generateDouble() - 0.5) * myPenWidth * 2);
        p.drawPoint(pt + QPoint(dx, dy));
    }
}

// ─────────────────────────────────────────────
//  Calligraphy
// ─────────────────────────────────────────────
void ScribbleArea::drawCalligraphy(QPainter &p, const QPoint &pt, const QPoint &prev)
{
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QColor c = myPenColor;
    c.setAlpha(200);
    int nib = myPenWidth;
    QPoint offset(nib, -nib);
    QPolygon poly;
    poly << prev << (prev + offset) << (pt + offset) << pt;
    QPainterPath path;
    path.addPolygon(poly);
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawPath(path);
}

// ─────────────────────────────────────────────
//  Fur
// ─────────────────────────────────────────────
void ScribbleArea::drawFur(QPainter &p, const QPoint &pt, const QPoint &prev)
{
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QColor c = myPenColor;
    c.setAlpha(200);
    p.setPen(QPen(c, 1, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(prev, pt);
    int strands = myPenWidth + 4;
    for (int i = 0; i < strands; ++i) {
        double angle = rng.generateDouble() * 2 * M_PI;
        double len   = rng.generateDouble() * myPenWidth * 1.5 + 2;
        int ex = pt.x() + static_cast<int>(len * std::cos(angle));
        int ey = pt.y() + static_cast<int>(len * std::sin(angle));
        c.setAlpha(static_cast<int>(rng.generateDouble() * 150) + 50);
        p.setPen(QPen(c, 1));
        p.drawLine(pt, QPoint(ex, ey));
    }
}

// ─────────────────────────────────────────────
//  Chalk
// ─────────────────────────────────────────────
void ScribbleArea::drawChalk(QPainter &p, const QPoint &pt, const QPoint &prev)
{
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QColor c = myPenColor;
    QPoint delta = pt - prev;
    double len = std::sqrt(static_cast<double>(delta.x()*delta.x() + delta.y()*delta.y()));
    int steps = qMax(1, static_cast<int>(len));
    for (int s = 0; s <= steps; s += 2) {
        double t = (steps > 0) ? (double)s / steps : 0;
        QPoint cur = prev + QPoint(static_cast<int>(delta.x()*t),
                                   static_cast<int>(delta.y()*t));
        for (int i = 0; i < myPenWidth * 2; ++i) {
            int ox = static_cast<int>((rng.generateDouble() - 0.5) * myPenWidth * 1.8);
            int oy = static_cast<int>((rng.generateDouble() - 0.5) * myPenWidth * 1.8);
            c.setAlpha(static_cast<int>(rng.generateDouble() * 120) + 40);
            p.setPen(QPen(c, 1));
            p.drawPoint(cur + QPoint(ox, oy));
        }
    }
}

// ─────────────────────────────────────────────
//  Utilities
// ─────────────────────────────────────────────
Qt::PenStyle ScribbleArea::qtPenStyle() const
{
    switch (myBrushStyle) {
    case Dashed: return Qt::DashLine;
    case Dotted: return Qt::DotLine;
    default:     return Qt::SolidLine;
    }
}

void ScribbleArea::resizeImage(QImage *img, const QSize &newSize)
{
    if (img->size() == newSize) return;
    QImage newImg(newSize, QImage::Format_RGB32);
    newImg.fill(qRgb(255, 255, 255));
    QPainter painter(&newImg);
    painter.drawImage(QPoint(0,0), *img);
    *img = newImg;
}

void ScribbleArea::floodFill(const QPoint &pos, const QColor &fillColor)
{
    if (!image.rect().contains(pos)) return;
    QRgb target = image.pixel(pos);
    QRgb fill   = fillColor.rgb();
    if (target == fill) return;

    QQueue<QPoint> queue;
    queue.enqueue(pos);

    while (!queue.isEmpty()) {
        QPoint p = queue.dequeue();
        if (!image.rect().contains(p)) continue;
        if (image.pixel(p) != target) continue;

        int x = p.x();
        while (x >= 0 && image.pixel(x, p.y()) == target) x--;
        x++;

        bool above = false, below = false;
        while (x < image.width() && image.pixel(x, p.y()) == target) {
            image.setPixel(x, p.y(), fill);
            if (!above && p.y() > 0 && image.pixel(x, p.y()-1) == target) {
                queue.enqueue(QPoint(x, p.y()-1)); above = true;
            } else if (above && p.y() > 0 && image.pixel(x, p.y()-1) != target) {
                above = false;
            }
            if (!below && p.y() < image.height()-1 && image.pixel(x, p.y()+1) == target) {
                queue.enqueue(QPoint(x, p.y()+1)); below = true;
            } else if (below && p.y() < image.height()-1 && image.pixel(x, p.y()+1) != target) {
                below = false;
            }
            x++;
        }
    }
    previewImage = image;
}
