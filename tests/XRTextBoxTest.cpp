#include "TextBox.h"

#include <QGuiApplication>
#include <QImage>
#include <QPainter>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    TextLineGroup lines(4);
    lines.AddLines("Qt text&highlighted", true);
    if (lines.GetLineCount() != 2) return 1;

    TextBox box(160, 64, RGB(0, 0, 0), RGB(255, 0, 0), CWHITE, 4, lines);
    QImage image(160, 64, QImage::Format_RGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QFont font("Sans", 12);
    if (!box.Render(&painter, 0, &font, 20, false)) return 2;
    if (box.Render(&painter, 0, &font, 20, false)) return 3;
    painter.end();

    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
            if (image.pixelColor(x, y) != QColor(Qt::white)) return 0;
    return 4;
}
