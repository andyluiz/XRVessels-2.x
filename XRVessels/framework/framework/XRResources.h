#pragma once

#ifdef _WIN32
#include <windows.h>
using XRBitmap = HBITMAP;

inline HBITMAP XRLoadBitmap(HINSTANCE module, int resourceID)
{
    return LoadBitmap(module, MAKEINTRESOURCE(resourceID));
}

inline void XRDeleteBitmap(XRBitmap image) { DeleteObject(image); }
#else
#include "OrbiterResource.h"
#include <QImage>
#include <QString>

using XRBitmap = QImage *;

inline QImage *XRLoadBitmap(void *module, int resourceID)
{
    return oapiLoadResImage(module, resourceID);
}

inline void XRDeleteBitmap(XRBitmap image) { delete image; }
#endif
