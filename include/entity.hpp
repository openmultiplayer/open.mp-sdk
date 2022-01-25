#pragma once

#include "component.hpp"
#include "events.hpp"
#include "gtaquat.hpp"
#include "types.hpp"
#include <set>

/// An interface for ID providers
struct IIDProvider {
    virtual int getID() const = 0;
};

/// An extra data interface for per-entity data
struct IExtraData : public IUIDProvider {
    /// Frees the extra data object, called on provider destruction, usually defaults to delete this
    virtual void free() = 0;
};

struct IExtraDataProvider {
    /// Add data associated with the provider, preferrably used on provider creation
    virtual void addData(IExtraData* extraData) = 0;

    /// Query provider data by its ID
    /// @param id The UID of the data
    /// @return A pointer to the data or nullptr if not available
    virtual IExtraData* findData(UID id) const = 0;
};

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

    ~ExtraDataProvider() {
        for (auto& v : extraData_) {
            v.second->free();
        }
    }

    FlatHashMap<UID, IExtraData*> extraData_;
};
}

/// Query extra data by its type
/// @typeparam ExtraDataT The data type, must derive from IExtraData
template <class ExtraDataT>
ExtraDataT* queryData(IExtraDataProvider* provider)
{
    static_assert(std::is_base_of<IExtraData, ExtraDataT>::value, "queryData parameter must inherit from IExtraData");
    return static_cast<ExtraDataT*>(provider->findData(ExtraDataT::IID));
}

/// Query extra data by its type
/// @typeparam ExtraDataT The data type, must derive from IExtraData
template <class ExtraDataT>
ExtraDataT* queryData(IExtraDataProvider& provider)
{
    static_assert(std::is_base_of<IExtraData, ExtraDataT>::value, "queryData parameter must inherit from IExtraData");
    return static_cast<ExtraDataT*>(provider.findData(ExtraDataT::IID));
}

/// A base entity interface
struct IEntity : virtual IExtensible, public IIDProvider, public IExtraDataProvider {
    /// Get the entity's position
    virtual Vector3 getPosition() const = 0;

    /// Set the entity's position
    virtual void setPosition(Vector3 position) = 0;

    /// Get the entity's rotation
    virtual GTAQuat getRotation() const = 0;

    /// Set the entity's rotation
    virtual void setRotation(GTAQuat rotation) = 0;

    /// Get the entity's virtual world
    virtual int getVirtualWorld() const = 0;

    /// Set the entity's virtual world
    virtual void setVirtualWorld(int vw) = 0;
};
