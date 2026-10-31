#include "XRModuleFlag.h"

#include <dlfcn.h>

// Exported from the executable: an unflagged vessel must not inherit this flag.
extern "C" {
bool XRVesselCtrlFlag = true;
}

static int Check(const char *filename, bool expected)
{
    void *module = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
    if (!module) return 1;
    auto create = reinterpret_cast<void *(*)()>(dlsym(module, "XRCreateFixture"));
    if (!create) { dlclose(module); return 2; }
    const void *vessel = create();
    const bool actual = XRModuleHasFlag(vessel, "XRVesselCtrlFlag");
    const bool missing = XRModuleHasFlag(vessel, "NoSuchFlag");
    dlclose(module);
    return actual == expected && !missing ? 0 : 3;
}

int main()
{
    if (XRModuleHasFlag(nullptr, "XRVesselCtrlFlag")) return 1;
    if (Check(XR_FLAGGED_LIBRARY, true)) return 2;
    if (Check(XR_UNFLAGGED_LIBRARY, false)) return 3;
    if (Check(XR_DISABLED_LIBRARY, false)) return 4;
    return 0;
}
