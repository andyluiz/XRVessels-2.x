#pragma once

#ifdef _WIN32
#include <windows.h>
using XRDialogHandle = HWND;
#else
#include <QAbstractButton>
#include <QWidget>

using XRDialogHandle = QWidget *;

inline QAbstractButton *XRDialogButton(QWidget *dialog, const char *name)
{
    return dialog ? dialog->findChild<QAbstractButton *>(QString::fromLatin1(name)) : nullptr;
}

inline void XRSetDialogCheck(QWidget *dialog, const char *name, bool checked)
{
    if (auto *button = XRDialogButton(dialog, name)) button->setChecked(checked);
}

template <typename Action>
void XRConnectDialogButton(QWidget *dialog, const char *name, Action action)
{
    if (auto *button = XRDialogButton(dialog, name))
        QObject::connect(button, &QAbstractButton::clicked, dialog, action);
}
#endif
