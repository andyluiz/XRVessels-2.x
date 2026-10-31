#include "XRQtDrawing.h"

#include <QGuiApplication>
#include <QImage>
#include <cstdio>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    const QString expected = QString::fromUtf8("21\xC2\xB0" "C");
    if (XRPanelText("21\xB0" "C", 4) != expected ||
        XRPanelText("21\xC2\xB0" "C", 5) != expected ||
        XRPanelText("21\xB0" "C", 3) != expected.left(3)) return 8;
    QImage image(160, 80, QImage::Format_RGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    const QFont original = painter.font();
    const QPen originalPen = painter.pen();
    HFONT font = CreateFont(-14, 0, 0, 0, 700, 0, 0, 0, 0, 0, 0,
                            NONANTIALIASED_QUALITY, 0, "Sans");
    HFONT small = CreateFont(10, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, "Sans");
    if (font->pixelSize() != 14 || font->weight() != QFont::Bold) return 1;
    HFONT cellFont = CreateFont(12, 0, 0, 0, 600, 0, 0, 0, 0, 0, 0, 0, FF_MODERN, "Microsoft Sans Serif");
    HFONT labelFont = CreateFont(10, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
    if (QFontMetrics(*cellFont).height() > 12 || QFontMetrics(*labelFont).height() > 10 ||
        46 + QFontMetrics(*cellFont).horizontalAdvance("DISENGAGED") >
            165 - QFontMetrics(*labelFont).horizontalAdvance("SET PITCH")) {
        std::fprintf(stderr, "font heights %d/%d; label edges %d/%d\n", QFontMetrics(*cellFont).height(), QFontMetrics(*labelFont).height(),
                     46 + QFontMetrics(*cellFont).horizontalAdvance("DISENGAGED"), 165 - QFontMetrics(*labelFont).horizontalAdvance("SET PITCH"));
        return 9;
    }
    DeleteObject(cellFont);
    DeleteObject(labelFont);
    HFONT previous = SelectObject(&painter, font);
    SelectObject(&painter, small);
    // Repeated selections must not invalidate the original borrowed font.
    for (int i = 0; i < 100; ++i) SelectObject(&painter, font);
    SelectObject(&painter, previous);
    if (painter.font() != original) return 2;

    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    HPEN previousPen = SelectObject(&painter, pen);
    MoveToEx(&painter, 2, 2, nullptr);
    LineTo(&painter, 20, 2);
    SelectObject(&painter, previousPen);
    if (painter.pen() != originalPen || image.pixelColor(10, 2) != QColor(Qt::red)) return 3;

    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 255));
    RECT rect{2, 5, 12, 10};
    FillRect(&painter, &rect, brush);
    if (image.pixelColor(5, 7) != QColor(Qt::blue) || image.pixelColor(12, 7) != QColor(Qt::white)) return 4;

    SelectObject(&painter, font);
    SetBkMode(&painter, TRANSPARENT);
    SetTextColor(&painter, RGB(0, 128, 0));
    SetTextAlign(&painter, TA_RIGHT);
    TextOut(&painter, 100, 20, "Test", 4);
    bool textDrawn = false;
    for (int y = 20; y < 45; ++y)
        for (int x = 0; x < 120; ++x)
            if (image.pixelColor(x, y) != QColor(Qt::white)) {
                if (x >= 100) return 5;
                textDrawn = true;
            }
    if (!textDrawn) return 6;
    xrqt::release(&painter);
    if (xrqt::states.count(&painter)) return 7;
    DeleteObject(font);
    DeleteObject(small);
    DeleteObject(pen);
    DeleteObject(brush);
    return 0;
}
