#include "XRTemplates.h"

#include <map>
#include <string>
#include <unordered_map>
#include "stringhasher.h"

struct Tracked
{
    explicit Tracked(int &count) : destroyed(count) {}
    ~Tracked() { ++destroyed; }
    int &destroyed;
};

int main()
{
    int keysDestroyed = 0, valuesDestroyed = 0;
    std::map<const Tracked *, Tracked *> cache;
    cache.emplace(new Tracked(keysDestroyed), new Tracked(valuesDestroyed));
    auto entry = cache.begin();
    EraseIteratorItemFirstSecond(cache, entry);
    if (!cache.empty() || keysDestroyed != 1 || valuesDestroyed != 1) return 1;

    std::unordered_map<const std::string *, Tracked *, stringhasher, stringhasher> payloadCache;
    payloadCache.emplace(new std::string("XRPayloadBay"), new Tracked(valuesDestroyed));
    payloadCache.emplace(new std::string("XR2PayloadCHM"), new Tracked(valuesDestroyed));
    const auto cleanup = [](const std::string *key, Tracked *value) { delete key; delete value; };
    ClearOwnedPointerMap(payloadCache, cleanup);
    ClearOwnedPointerMap(payloadCache, cleanup); // repeat termination is safe
    if (!payloadCache.empty() || valuesDestroyed != 3) return 2;
    payloadCache.emplace(new std::string("XR2PayloadCHM"), new Tracked(valuesDestroyed));
    const std::string name("XR2PayloadCHM");
    if (payloadCache.find(&name) == payloadCache.end()) return 3;
    ClearOwnedPointerMap(payloadCache, cleanup);
    return valuesDestroyed == 4 ? 0 : 4;
}
