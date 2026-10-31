#include "XRRenderWindow.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QWindow unrelated;
    unrelated.show();
    QWindow render;
    render.show();

    if (XRRenderWindow()) return 1;
    QObject marker(&render);
    marker.setObjectName("Orbiter Render Window");
    if (XRRenderWindow() != &render) return 2;

    render.setWindowState(Qt::WindowFullScreen);
    if (XRRenderWindow()) return 3;
    render.setWindowState(Qt::WindowNoState);
    if (XRRenderWindow() != &render) return 4;
    return 0;
}
