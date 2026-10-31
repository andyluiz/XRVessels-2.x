#include "XRControlDialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QRadioButton>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QWidget dialog;
    QRadioButton door(&dialog);
    door.setObjectName("IDC_GEAR_UP");
    QCheckBox light(&dialog);
    light.setObjectName("IDC_NAVLIGHT");
    int commands = 0;
    bool lightOn = false;
    XRConnectDialogButton(&dialog, "IDC_GEAR_UP", [&] { ++commands; });
    XRConnectDialogButton(&dialog, "IDC_NAVLIGHT", [&](bool checked) { lightOn = checked; });
    XRSetDialogCheck(&dialog, "IDC_GEAR_UP", true);
    if (!door.isChecked() || commands) return 1;
    door.click();
    light.click();
    if (commands != 1 || !lightOn) return 2;
    XRSetDialogCheck(&dialog, "IDC_NAVLIGHT", false);
    if (light.isChecked() || !lightOn) return 3;
    return 0;
}
