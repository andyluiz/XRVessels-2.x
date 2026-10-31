#pragma once

#ifndef _WIN32
#include <QGuiApplication>
#include <QWindow>

// The Linux Orbiter graphics client attaches its render event filter as a
// child of the render QWindow with this object name (GraphicsAPI.cpp).
inline QWindow *XRRenderWindow()
{
    if (!QGuiApplication::instance()) return nullptr;
    for (QWindow *window : QGuiApplication::allWindows())
    {
        if (window && window->windowState() != Qt::WindowFullScreen &&
            window->findChild<QObject *>("Orbiter Render Window"))
            return window;
    }
    return nullptr;
}
#endif
