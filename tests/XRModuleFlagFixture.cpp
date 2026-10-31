class VesselFixture
{
public:
    virtual ~VesselFixture() = default;
    virtual int Version() const { return 1; }
};

extern "C" __attribute__((visibility("default"))) void *XRCreateFixture()
{
    static VesselFixture vessel;
    return &vessel;
}

#ifdef XR_FLAGGED_FIXTURE
extern "C" {
__attribute__((visibility("default"))) bool XRVesselCtrlFlag = true;
}
#endif

#ifdef XR_DISABLED_FIXTURE
extern "C" {
__attribute__((visibility("default"))) bool XRVesselCtrlFlag = false;
}
#endif
