#pragma once

#ifdef _WIN32
#include <windows.h>
using XRPanelPen = HPEN;
using XRPanelBrush = HBRUSH;
#else
#include "XRQtDrawing.h"
#include <QBrush>
#include <QPainter>
#include <QPen>
using HDC = QPainter *;
using XRPanelPen = QPen *;
using XRPanelBrush = QBrush *;
#endif
