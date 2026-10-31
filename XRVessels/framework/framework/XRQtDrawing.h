#pragma once

// The small GDI drawing subset used by XR panels, implemented on Orbiter's
// QPainter surfaces. Windows continues to use its native GDI implementation.
#ifndef _WIN32
#include "OrbiterPlatform.h"
#include "XRPanelText.h"
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPen>
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <unordered_map>

using HDC = QPainter *;
using HFONT = QFont *;
using HPEN = QPen *;
using HBRUSH = QBrush *;
constexpr int TA_LEFT = 0, TA_RIGHT = 2, TA_CENTER = 6;
constexpr int TRANSPARENT = 1, OPAQUE = 2, PS_SOLID = 0, FF_MODERN = 48;
constexpr int ANTIALIASED_QUALITY = 4, NONANTIALIASED_QUALITY = 3, DEFAULT_QUALITY = 0;

namespace xrqt {
struct DrawingState {
    std::deque<QFont> fonts;
    std::deque<QPen> pens;
    int alignment = TA_LEFT;
    QPoint position;
    QColor textColor = Qt::black;
};
inline std::unordered_map<HDC, DrawingState> states;
inline QColor color(COLORREF c) { return QColor(GetRValue(c), GetGValue(c), GetBValue(c)); }
// Previous-object pointers are valid until Area::ReleaseDC, just like borrowed
// GDI objects. deque keeps them stable across multiple font/pen selections.
inline void release(HDC dc) { states.erase(dc); }
}

inline HFONT CreateFont(int height, int, int, int, int weight, unsigned italic,
    unsigned underline, unsigned strikeout, unsigned, unsigned, unsigned,
    unsigned quality, unsigned, const char *face)
{
    QString family = QString::fromUtf8(face);
    // Microsoft Sans Serif is usually absent on Linux; Arial's Linux font
    // substitution is a closer match than the default Noto Sans fallback.
    if (family.compare("Microsoft Sans Serif", Qt::CaseInsensitive) == 0) family = "Arial";
    auto *font = new QFont(family);
    font->setPixelSize(std::max(1, std::abs(height)));
    font->setWeight(static_cast<QFont::Weight>(std::clamp(weight, 1, 1000)));
    font->setItalic(italic); font->setUnderline(underline); font->setStrikeOut(strikeout);
    // A positive GDI height specifies the whole character cell; a negative
    // height specifies the em size. Qt's pixel size is the latter.
    if (height > 0) {
        while (font->pixelSize() > 1 && QFontMetrics(*font).height() > height)
            font->setPixelSize(font->pixelSize() - 1);
        // Reserve the internal leading included in Windows' character cell.
        if (font->pixelSize() > 1) font->setPixelSize(font->pixelSize() - 1);
    }
    if (quality == NONANTIALIASED_QUALITY) font->setStyleStrategy(QFont::NoAntialias);
    else if (quality == ANTIALIASED_QUALITY) font->setStyleStrategy(QFont::PreferAntialias);
    return font;
}
inline HPEN CreatePen(int, int width, COLORREF color) { return new QPen(xrqt::color(color), width); }
inline HBRUSH CreateSolidBrush(COLORREF color) { return new QBrush(xrqt::color(color)); }
inline void DeleteObject(HFONT object) { delete object; }
inline void DeleteObject(HPEN object) { delete object; }
inline void DeleteObject(HBRUSH object) { delete object; }

inline HFONT SelectObject(HDC dc, HFONT font)
{
    auto &saved = xrqt::states[dc].fonts;
    saved.push_back(dc->font());
    if (font) dc->setFont(*font);
    return &saved.back();
}
inline HPEN SelectObject(HDC dc, HPEN pen)
{
    auto &saved = xrqt::states[dc].pens;
    saved.push_back(dc->pen());
    if (pen) dc->setPen(*pen);
    return &saved.back();
}
inline void SetTextAlign(HDC dc, int alignment) { xrqt::states[dc].alignment = alignment; }
inline void SetTextColor(HDC dc, COLORREF color) { xrqt::states[dc].textColor = xrqt::color(color); }
inline void SetBkColor(HDC dc, COLORREF color) { dc->setBackground(xrqt::color(color)); }
inline void SetBkMode(HDC dc, int mode) { dc->setBackgroundMode(mode == TRANSPARENT ? Qt::TransparentMode : Qt::OpaqueMode); }
inline void TextOut(HDC dc, int x, int y, const char *text, int length)
{
    const auto &state = xrqt::states[dc];
    const QString value = XRPanelText(text, length);
    const int width = dc->fontMetrics().horizontalAdvance(value);
    if (state.alignment == TA_RIGHT) x -= width;
    else if (state.alignment == TA_CENTER) x -= width / 2;
    dc->save();
    dc->setPen(state.textColor);
    dc->drawText(x, y + dc->fontMetrics().ascent(), value);
    dc->restore();
}
inline void FillRect(HDC dc, const RECT *rect, HBRUSH brush)
{
    dc->fillRect(QRect(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top), *brush);
}
inline void MoveToEx(HDC dc, int x, int y, POINT *previous)
{
    auto &point = xrqt::states[dc].position;
    if (previous) { previous->x = point.x(); previous->y = point.y(); }
    point = QPoint(x, y);
}
inline void LineTo(HDC dc, int x, int y)
{
    auto &point = xrqt::states[dc].position;
    dc->drawLine(point, QPoint(x, y));
    point = QPoint(x, y);
}
#endif
