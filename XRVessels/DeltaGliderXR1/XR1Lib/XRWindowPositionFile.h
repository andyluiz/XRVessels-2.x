#pragma once

#ifndef _WIN32

#include <QPoint>
#include <QSettings>
#include <QString>

// Linux replacement for the XR render-window position values in the registry.
// Stored as a human-editable INI file under Orbiter's Config directory.
class XRWindowPositionFile
{
public:
    explicit XRWindowPositionFile(const QString &filename) : m_settings(filename, QSettings::IniFormat) {}

    bool Save(unsigned int width, unsigned int height, const QPoint &position)
    {
        m_settings.setValue(XKey(width), position.x());
        m_settings.setValue(YKey(height), position.y());
        m_settings.sync();
        return m_settings.status() == QSettings::NoError;
    }

    bool Load(unsigned int width, unsigned int height, QPoint &position)
    {
        bool xValid = false, yValid = false;
        const int x = m_settings.value(XKey(width)).toInt(&xValid);
        const int y = m_settings.value(YKey(height)).toInt(&yValid);
        if (!xValid || !yValid || m_settings.status() != QSettings::NoError)
            return false;

        position = QPoint(x, y);
        return true;
    }

    bool RestoreDisabled() const
    {
        return m_settings.value("DisableWindowPosRestore", false).toBool();
    }

private:
    static QString XKey(unsigned int width) { return "x_window_coord_" + QString::number(width); }
    static QString YKey(unsigned int height) { return "y_window_coord_" + QString::number(height); }

    QSettings m_settings;
};

#endif
