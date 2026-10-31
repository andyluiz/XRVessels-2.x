#pragma once

#ifndef _WIN32
#include <dlfcn.h>

// Locate the module owning a vessel's vtable, then check that module's own
// export. This does not require RTTI (third-party vessels may disable it).
inline bool XRModuleHasFlag(const void *vessel, const char *name)
{
    if (!vessel || !name) return false;

    // Orbiter's Linux vessel modules use the GCC C++ ABI: the first word of a
    // polymorphic object is the pointer to its vtable in the vessel module.
    const void *vtable = *static_cast<const void *const *>(vessel);
    Dl_info module{};
    if (!vtable || !dladdr(vtable, &module) || !module.dli_fname) return false;

    void *handle = dlopen(module.dli_fname, RTLD_NOW | RTLD_NOLOAD);
    if (!handle) return false;

    const void *flag = dlsym(handle, name);
    Dl_info owner{};
    // dlsym may find an export in a dependency: only trust the vessel module.
    const bool enabled = flag && dladdr(flag, &owner) &&
        owner.dli_fbase == module.dli_fbase && *static_cast<const bool *>(flag);
    dlclose(handle);
    return enabled;
}
#endif
