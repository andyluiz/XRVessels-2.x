#pragma once

#ifndef _WIN32
#include <QString>
#include <QStringDecoder>
#include <QByteArrayView>

// XR's original Windows strings use single-byte degree symbols, while
// Linux input may also contain UTF-8. Prefer UTF-8 when it is valid.
inline QString XRPanelText(const char *text, int length)
{
    QStringDecoder decoder(QStringDecoder::Utf8);
    const QString decoded = decoder(QByteArrayView(text, length));
    return decoder.hasError() ? QString::fromLatin1(text, length) : decoded;
}
#endif
