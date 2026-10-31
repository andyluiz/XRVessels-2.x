#include "XRWindowPositionFile.h"

#include <QSettings>
#include <QTemporaryDir>

int main()
{
    QTemporaryDir directory;
    if (!directory.isValid()) return 1;
    const QString filename = directory.filePath("XRWindowPosition.ini");

    {
        XRWindowPositionFile file(filename);
        QPoint position(42, 43);
        if (file.Load(1280, 720, position) || position != QPoint(42, 43)) return 2;
        if (file.RestoreDisabled()) return 3;
        if (!file.Save(1280, 720, QPoint(-24, 36))) return 4;
        if (!file.Save(1920, 1080, QPoint(17, -44))) return 5;
    }

    {
        XRWindowPositionFile file(filename);
        QPoint position;
        if (!file.Load(1280, 720, position) || position != QPoint(-24, 36)) return 6;
        if (!file.Load(1920, 1080, position) || position != QPoint(17, -44)) return 7;
        if (file.Load(320, 1080, position)) return 8;
    }

    {
        QSettings settings(filename, QSettings::IniFormat);
        settings.setValue("DisableWindowPosRestore", 1);
        settings.sync();
        XRWindowPositionFile file(filename);
        if (!file.RestoreDisabled()) return 9;
    }

    XRWindowPositionFile unwritable("/dev/null/XRWindowPosition.ini");
    if (unwritable.Save(1280, 720, QPoint(1, 2))) return 10;
    return 0;
}
