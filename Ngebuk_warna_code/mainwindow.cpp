#include <QtWidgets>
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmap>
#include "mainwindow.h"
#include "scribblearea.h"
#include "brushpanel.h"

// Brush display names (matching BrushType enum order)
static const char* BRUSH_NAMES[] = {
    "Normal", "Spray", "Marker", "Glow",
    "Watercolor", "Kaligrafi", "Fur", "Chalk"
};

// Helper: render SVG resource to QIcon
static QIcon svgIcon(const QString &resourcePath, int size = 24)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QSvgRenderer renderer(resourcePath);
    if (renderer.isValid()) {
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        renderer.render(&p);
    }
    return QIcon(pix);
}

MainWindow::MainWindow()
{
    scribbleArea = new ScribbleArea;
    setCentralWidget(scribbleArea);

    brushPanel = new BrushPanel(this);
    connect(brushPanel, &BrushPanel::brushSelected,
            this, &MainWindow::onBrushSelected);

    createActions();
    createMenus();
    createToolbar();

    setWindowTitle(tr("Ngebuk Warna"));
    resize(1100, 720);
}

// ─────────────────────────────────────────────
void MainWindow::createActions()
{
    openAct = new QAction(
        style()->standardIcon(QStyle::SP_DirOpenIcon), tr("&Open"), this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    saveJpgAct = new QAction(
        style()->standardIcon(QStyle::SP_DialogSaveButton), tr("&Save JPG"), this);
    saveJpgAct->setShortcut(QKeySequence::Save);
    connect(saveJpgAct, &QAction::triggered, this, &MainWindow::saveAsJpg);

    undoAct = new QAction(
        style()->standardIcon(QStyle::SP_ArrowBack), tr("&Undo"), this);
    undoAct->setShortcut(QKeySequence::Undo);
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);

    redoAct = new QAction(
        style()->standardIcon(QStyle::SP_ArrowForward), tr("&Redo"), this);
    redoAct->setShortcut(QKeySequence::Redo);
    connect(redoAct, &QAction::triggered, this, &MainWindow::redo);

    clearAct = new QAction(
        style()->standardIcon(QStyle::SP_TrashIcon), tr("&Clear"), this);
    clearAct->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_Delete);
    connect(clearAct, &QAction::triggered, scribbleArea, &ScribbleArea::clearImage);

    colorAct = new QAction(svgIcon(":/icons/colorpicker.svg"), tr("Warna"), this);
    connect(colorAct, &QAction::triggered, this, &MainWindow::penColor);

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    // ── Tool group ──
    toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);
    connect(toolGroup, &QActionGroup::triggered, this, &MainWindow::onToolChanged);

    pencilAct = new QAction(svgIcon(":/icons/pencil.svg"), tr("Pensil"), this);
    pencilAct->setCheckable(true);
    pencilAct->setChecked(true);
    pencilAct->setData((int)ScribbleArea::Pencil);
    pencilAct->setShortcut(Qt::Key_P);
    pencilAct->setToolTip(tr("Pensil (P) – klik lagi untuk pilih jenis brush"));
    toolGroup->addAction(pencilAct);

    eraserAct = new QAction(svgIcon(":/icons/eraser.svg"), tr("Eraser"), this);
    eraserAct->setCheckable(true);
    eraserAct->setData((int)ScribbleArea::Eraser);
    eraserAct->setShortcut(Qt::Key_E);
    toolGroup->addAction(eraserAct);

    fillAct = new QAction(svgIcon(":/icons/fill.svg"), tr("Fill"), this);
    fillAct->setCheckable(true);
    fillAct->setData((int)ScribbleArea::FloodFill);
    fillAct->setShortcut(Qt::Key_F);
    toolGroup->addAction(fillAct);

    rectAct = new QAction(svgIcon(":/icons/rect.svg"), tr("Rect"), this);
    rectAct->setCheckable(true);
    rectAct->setData((int)ScribbleArea::DrawRect);
    rectAct->setShortcut(Qt::Key_R);
    toolGroup->addAction(rectAct);

    ellipseAct = new QAction(svgIcon(":/icons/ellipse.svg"), tr("Ellipse"), this);
    ellipseAct->setCheckable(true);
    ellipseAct->setData((int)ScribbleArea::DrawEllipse);
    ellipseAct->setShortcut(Qt::Key_O);
    toolGroup->addAction(ellipseAct);

    lineAct = new QAction(svgIcon(":/icons/line.svg"), tr("Line"), this);
    lineAct->setCheckable(true);
    lineAct->setData((int)ScribbleArea::DrawLine);
    lineAct->setShortcut(Qt::Key_L);
    toolGroup->addAction(lineAct);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveJpgAct);
    fileMenu->addSeparator();
    fileMenu->addAction(clearAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolbar()
{
    toolBar = addToolBar(tr("Tools"));
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(28, 28));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    toolBar->addAction(openAct);
    toolBar->addAction(saveJpgAct);
    toolBar->addSeparator();
    toolBar->addAction(undoAct);
    toolBar->addAction(redoAct);
    toolBar->addAction(clearAct);
    toolBar->addSeparator();

    // ── Tool buttons ──
    toolBar->addAction(pencilAct);
    toolBar->addAction(eraserAct);
    toolBar->addAction(fillAct);
    toolBar->addAction(rectAct);
    toolBar->addAction(ellipseAct);
    toolBar->addAction(lineAct);
    toolBar->addSeparator();

    // ── Color ──
    toolBar->addAction(colorAct);
    colorPreview = new QLabel;
    colorPreview->setFixedSize(28, 28);
    colorPreview->setStyleSheet(
        "background-color: black; border: 2px solid #888; border-radius: 4px;");
    colorPreview->setToolTip(tr("Warna aktif"));
    toolBar->addWidget(colorPreview);
    toolBar->addSeparator();

    // ── Line style ──
    toolBar->addWidget(new QLabel(" Style:"));
    brushStyleCombo = new QComboBox;
    brushStyleCombo->addItem("Solid",  0);
    brushStyleCombo->addItem("Dashed", 1);
    brushStyleCombo->addItem("Dotted", 2);
    brushStyleCombo->setFixedWidth(76);
    connect(brushStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onBrushStyleChanged);
    toolBar->addWidget(brushStyleCombo);
    toolBar->addSeparator();

    // ── Size slider ──
    toolBar->addWidget(new QLabel(" Ukuran:"));
    sizeSlider = new QSlider(Qt::Horizontal);
    sizeSlider->setRange(1, 50);
    sizeSlider->setValue(8);
    sizeSlider->setFixedWidth(110);
    connect(sizeSlider, &QSlider::valueChanged, this, &MainWindow::onBrushSizeChanged);
    toolBar->addWidget(sizeSlider);
    sizeLabel = new QLabel("8");
    sizeLabel->setFixedWidth(22);
    toolBar->addWidget(sizeLabel);
}

// ─────────────────────────────────────────────
//  Slots
// ─────────────────────────────────────────────

void MainWindow::onToolChanged(QAction *action)
{
    int toolVal = action->data().toInt();
    auto tool = static_cast<ScribbleArea::Tool>(toolVal);
    scribbleArea->setTool(tool);

    // If pencil was clicked while already selected → show brush panel
    if (tool == ScribbleArea::Pencil) {
        showBrushPanel();
    }
}

// Called when pencil is clicked even if already checked
// We override the triggered signal by checking the source
void MainWindow::showBrushPanel()
{
    // Find the pencil toolbar button widget
    QWidget *pencilWidget = nullptr;
    for (QWidget *w : toolBar->findChildren<QWidget*>()) {
        if (auto *tb = qobject_cast<QToolButton*>(w)) {
            if (tb->defaultAction() == pencilAct) {
                pencilWidget = tb;
                break;
            }
        }
    }

    brushPanel->setCurrentBrush(scribbleArea->brushType());
    brushPanel->setPreviewColor(scribbleArea->penColor());
    brushPanel->adjustSize();

    QPoint pos;
    if (pencilWidget) {
        pos = pencilWidget->mapToGlobal(QPoint(0, pencilWidget->height()));
    } else {
        pos = QCursor::pos();
    }
    brushPanel->move(pos);
    brushPanel->show();
    brushPanel->raise();
}

void MainWindow::onBrushSelected(ScribbleArea::BrushType type)
{
    scribbleArea->setBrushType(type);
    int idx = static_cast<int>(type);
    QString name = (idx >= 0 && idx < 8) ? QString(BRUSH_NAMES[idx]) : "Normal";
    pencilAct->setText(QString("Pensil [%1]").arg(name));

    pencilAct->setChecked(true);
    scribbleArea->setTool(ScribbleArea::Pencil);
}

void MainWindow::onBrushStyleChanged(int index)
{
    scribbleArea->setBrushStyle(static_cast<ScribbleArea::BrushStyle>(index));
}

void MainWindow::onBrushSizeChanged(int value)
{
    scribbleArea->setPenWidth(value);
    sizeLabel->setText(QString::number(value));
}

void MainWindow::penColor()
{
    QColor newColor = QColorDialog::getColor(
        scribbleArea->penColor(), this,
        tr("Pilih Warna"), QColorDialog::ShowAlphaChannel);

    if (newColor.isValid()) {
        scribbleArea->setPenColor(newColor);
        colorPreview->setStyleSheet(
            QString("background-color:%1; border:2px solid #888; border-radius:4px;")
                .arg(newColor.name()));
        brushPanel->setPreviewColor(newColor);
    }
}

void MainWindow::undo()  { scribbleArea->undo(); }
void MainWindow::redo()  { scribbleArea->redo(); }

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(
            this, tr("Buka Gambar"), QDir::currentPath(),
            tr("Images (*.png *.jpg *.jpeg *.bmp)"));
        if (!fileName.isEmpty())
            scribbleArea->openImage(fileName);
    }
}

void MainWindow::saveAsJpg()
{
    QString initialPath = QDir::homePath() + "/untitled.jpg";
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Simpan sebagai JPG"), initialPath,
        tr("JPEG Image (*.jpg *.jpeg)"));
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".jpg",  Qt::CaseInsensitive) &&
        !fileName.endsWith(".jpeg", Qt::CaseInsensitive))
        fileName += ".jpg";
    if (scribbleArea->saveImage(fileName, "jpg"))
        QMessageBox::information(this, tr("Tersimpan"),
            tr("Gambar disimpan ke:\n%1").arg(fileName));
    else
        QMessageBox::warning(this, tr("Gagal"),
            tr("Gambar gagal disimpan."));
}

bool MainWindow::maybeSave()
{
    if (!scribbleArea->isModified()) return true;
    auto ret = QMessageBox::warning(this, tr("Paint App"),
        tr("Gambar telah diubah.\nSimpan perubahan?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)   return saveFile("jpg");
    if (ret == QMessageBox::Cancel) return false;
    return true;
}

bool MainWindow::saveFile(const QByteArray &fmt)
{
    QString initialPath = QDir::homePath() + "/untitled." + fmt;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Simpan"), initialPath);
    if (fileName.isEmpty()) return false;
    return scribbleArea->saveImage(fileName, fmt.constData());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    maybeSave() ? event->accept() : event->ignore();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("Tentang Paint App"),
        tr("<h3>Paint App – Tugas Besar</h3>"
           "<p><b>Brush Types:</b></p>"
           "<ul>"
           "<li>✏ <b>Normal</b> – pensil solid biasa</li>"
           "<li>💨 <b>Spray</b> – semprotan titik acak</li>"
           "<li>🖊 <b>Marker</b> – spidol lebar semi-transparan</li>"
           "<li>✨ <b>Glow</b> – efek cahaya neon berpendar</li>"
           "<li>🎨 <b>Watercolor</b> – cat air lembut berbauran</li>"
           "<li>🖋 <b>Kaligrafi</b> – pena miring berkarakter</li>"
           "<li>🪶 <b>Fur</b> – bulu/rambut halus menyebar</li>"
           "<li>🪨 <b>Chalk</b> – kapur bertekstur kasar</li>"
           "</ul>"
           "<p><b>Shortcut:</b> tekan tombol Pensil (P) untuk buka panel brush</p>"));
}
