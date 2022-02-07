#include "../entity.hpp"

/* Implementation, NOT to be passed around */

namespace Impl {

struct ExtraDataProvider {
    IExtraData* findData(UID uuid) const
    {
        auto it = extraData_.find(uuid);
        return it == extraData_.end() ? nullptr : it->second;
    }

    void addData(IExtraData* playerData)
    {
        extraData_.try_emplace(playerData->getUID(), playerData);
    }

    void free()
    {
        for (auto& v : extraData_) {
            v.second->free();
        }
        extraData_.clear();
    }

    ~ExtraDataProvider()
    {
        free();
    }

private:
    FlatHashMap<UID, IExtraData*> extraData_;
};

}
