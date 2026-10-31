#ifndef _WIN32

#include "DeltaGliderXR1.h"
#include "XR1PayloadDialog.h"
#include "XRPayload.h"
#include "XRPayloadBaySlot.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QLabel>
#include <QTimer>

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace {
template <typename T> T *control(QWidget *dialog, const char *name)
{
    return dialog->findChild<T *>(QString::fromLatin1(name));
}

void setText(QWidget *dialog, const char *name, const char *text)
{
    if (auto *label = control<QLabel>(dialog, name)) label->setText(QString::fromUtf8(text));
}

QComboBox *payloadSelection(QWidget *dialog)
{
    return control<QComboBox>(dialog, "IDC_COMBO_SELECTED_PAYLOAD_OBJECT");
}

std::string selectedClass(QWidget *dialog)
{
    auto *combo = payloadSelection(dialog);
    return combo ? combo->currentText().toStdString() : std::string();
}
}

void XR1PayloadDialog::EditorFunc(OBJHANDLE vessel)
{
    Launch(vessel);
}

XRDialogHandle XR1PayloadDialog::Launch(OBJHANDLE vessel)
{
    auto *xr1 = static_cast<DeltaGliderXR1 *>(oapiGetVesselInterface(vessel));
    if (!xr1) return nullptr;
    return oapiOpenDialogEx(g_hDLL, GLOBAL_IDD_PAYLOAD_EDITOR, Initialize, DLG_CAPTIONCLOSE, xr1);
}

void XR1PayloadDialog::CloseDialog(XRDialogHandle dialog)
{
    if (!dialog) return;
    if (DeltaGliderXR1::s_hPayloadEditorDialog == dialog)
        DeltaGliderXR1::s_hPayloadEditorDialog = nullptr;
    oapiCloseDialog(dialog);  // child timers and connections are destroyed with the dialog
}

void XR1PayloadDialog::Initialize(QWidget *dialog, void *context)
{
    auto *xr1 = static_cast<DeltaGliderXR1 *>(context);
    if (!dialog || !xr1 || !xr1->m_pPayloadBay) return;
    QObject::connect(dialog, &QObject::destroyed, [dialog] {
        if (DeltaGliderXR1::s_hPayloadEditorDialog == dialog)
            DeltaGliderXR1::s_hPayloadEditorDialog = nullptr;
    });

    if (auto *combo = payloadSelection(dialog)) {
        const XRPayloadClassData **classes = XRPayloadClassData::GetAllAvailableXRPayloads();
        if (classes) for (; *classes; ++classes)
            combo->addItem(QString::fromUtf8((*classes)->GetClassname()));
        if (combo->property("resSort").toBool()) combo->model()->sort(0);
        combo->setCurrentIndex(combo->count() ? 0 : -1);
        combo->setFocus();
        QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), dialog,
            [dialog, xr1](int) { SelectedPayloadChanged(dialog, *xr1); });
    }

    const int count = std::min(slotCount, xr1->m_pPayloadBay->GetSlotCount());
    for (int i = 0; i < count; ++i) {
        const QString name = QStringLiteral("IDC_SLOT%1").arg(i + 1);
        if (auto *button = dialog->findChild<QAbstractButton *>(name))
            QObject::connect(button, &QAbstractButton::clicked, dialog, [dialog, xr1, i](bool checked) {
                const int slot = i + 1;
                const auto classname = selectedClass(dialog);
                const bool success = checked
                    ? (!classname.empty() && xr1->m_pPayloadBay->CreateAndAttachPayloadVessel(classname.c_str(), slot))
                    : xr1->m_pPayloadBay->DeleteAttachedPayloadVessel(slot);
                if (checked && !success)
                    xr1->PlaySound(DeltaGliderXR1::Error1, DeltaGliderXR1::ST_Other, ERROR1_VOL);
                else
                    xr1->PlaySound(checked ? xr1->SwitchOn : xr1->SwitchOff, DeltaGliderXR1::ST_Other, MED_CLICK);
                RescanBay(dialog, *xr1);  // also undo a rejected check
            });
    }

    if (auto *button = control<QAbstractButton>(dialog, "IDC_EMPTY_BAY"))
        QObject::connect(button, &QAbstractButton::clicked, dialog, [dialog, xr1] {
            xr1->m_pPayloadBay->DeleteAllAttachedPayloadVessels();
            RescanBay(dialog, *xr1);
            xr1->PlaySound(xr1->SwitchOff, DeltaGliderXR1::ST_Other, MED_CLICK);
        });
    if (auto *button = control<QAbstractButton>(dialog, "IDC_FILL_PAYLOAD_BAY"))
        QObject::connect(button, &QAbstractButton::clicked, dialog, [dialog, xr1] {
            const auto classname = selectedClass(dialog);
            if (!classname.empty()) xr1->m_pPayloadBay->CreateAndAttachPayloadVesselInAllSlots(classname.c_str());
            RescanBay(dialog, *xr1);
            xr1->PlaySound(xr1->SwitchOn, DeltaGliderXR1::ST_Other, MED_CLICK);
        });
    if (auto *button = control<QAbstractButton>(dialog, "IDC_SELECTED_REMOVE_ALL"))
        QObject::connect(button, &QAbstractButton::clicked, dialog, [dialog, xr1] {
            const auto classname = selectedClass(dialog);
            if (!classname.empty()) xr1->m_pPayloadBay->DeleteAllAttachedPayloadVesselsOfClassname(classname.c_str());
            RescanBay(dialog, *xr1);
            xr1->PlaySound(xr1->SwitchOff, DeltaGliderXR1::ST_Other, MED_CLICK);
        });
    if (auto *button = control<QAbstractButton>(dialog, "IDC_BACK"))
        QObject::connect(button, &QAbstractButton::clicked, dialog, [dialog] { CloseDialog(dialog); });

    RescanBay(dialog, *xr1);
    SelectedPayloadChanged(dialog, *xr1);
    auto *massTimer = new QTimer(dialog);
    QObject::connect(massTimer, &QTimer::timeout, dialog, [dialog, xr1] { UpdateMassValues(dialog, *xr1); });
    massTimer->start(50);
    auto *bayTimer = new QTimer(dialog);
    QObject::connect(bayTimer, &QTimer::timeout, dialog, [dialog, xr1] { RescanBay(dialog, *xr1); });
    bayTimer->start(200);
}

void XR1PayloadDialog::UpdateMassValues(QWidget *dialog, const DeltaGliderXR1 &vessel)
{
    char value[80];
    const double payloadMass = vessel.GetPayloadMass();
    std::snprintf(value, sizeof(value), "%10.1f kg (%10.1f lb)", payloadMass, payloadMass * 2.20462262);
    setText(dialog, "IDC_STATIC_PAYLOAD_MASS", value);
    const double vesselMass = vessel.GetMass();
    std::snprintf(value, sizeof(value), "%10.1f kg (%10.1f lb)", vesselMass, vesselMass * 2.20462262);
    setText(dialog, "IDC_STATIC_VESSEL_MASS", value);
}

void XR1PayloadDialog::UpdatePayloadFields(QWidget *dialog, const char *classname)
{
    const auto &payload = XRPayloadClassData::GetXRPayloadClassDataForClassname(classname);
    char value[80];
    setText(dialog, "IDC_STATIC_DESCRIPTION", payload.GetDescription());
    std::snprintf(value, sizeof(value), "%.3f", payload.GetMass());
    setText(dialog, "IDC_STATIC_MASS", value);
    const VECTOR3 dim = payload.GetDimensions();
    std::snprintf(value, sizeof(value), "%.2f L x %.2f W x %.2f H", dim.z, dim.x, dim.y);
    setText(dialog, "IDC_STATIC_DIMENSIONS", value);
    const VECTOR3 occupiedSlots = payload.GetSlotsOccupied();
    std::snprintf(value, sizeof(value), "%.1f L x %.1f W x %.1f H", occupiedSlots.z, occupiedSlots.x, occupiedSlots.y);
    setText(dialog, "IDC_STATIC_SLOTS_OCCUPIED", value);
    if (auto *label = control<QLabel>(dialog, "IDC_STATIC_THUMBNAIL_BMP")) {
        label->setFixedSize(PAYLOAD_THUMBNAIL_DIMX, PAYLOAD_THUMBNAIL_DIMY);
        const auto *image = payload.GetThumbnailBitmapHandle();
        label->setPixmap(image ? QPixmap::fromImage(*image) : QPixmap());
    }
}

void XR1PayloadDialog::RescanBay(QWidget *dialog, DeltaGliderXR1 &vessel)
{
    const auto *bay = static_cast<XR1PayloadBay *>(vessel.m_pPayloadBay);
    const auto classname = selectedClass(dialog);
    const int count = std::min(slotCount, bay->GetSlotCount());
    for (int i = 0; i < count; ++i) {
        const auto *slot = bay->GetSlot(i + 1);
        const QString name = QStringLiteral("IDC_SLOT%1").arg(i + 1);
        if (auto *button = dialog->findChild<QAbstractButton *>(name)) {
            button->setChecked(slot->IsOccupied());
            button->setEnabled(slot->IsEnabled());
            auto font = button->font();
            const auto *child = slot->GetChild();
            font.setBold(child && classname == child->GetClassName());
            button->setFont(font);
        }
    }
    UpdateMassValues(dialog, vessel);
}

void XR1PayloadDialog::SelectedPayloadChanged(QWidget *dialog, DeltaGliderXR1 &vessel)
{
    const auto classname = selectedClass(dialog);
    if (!classname.empty()) UpdatePayloadFields(dialog, classname.c_str());
    RescanBay(dialog, vessel);
}
#endif
