#pragma once

#include "types.hpp"

#ifndef BUILD_NUMBER
#define BUILD_NUMBER 0
#endif

/// Should always be used in classes inheriting IExtension
#define PROVIDE_EXT_UID(uuid)                 \
    static constexpr UID ExtensionIID = uuid; \
    UID getExtensionID() override { return ExtensionIID; }

/// A class which should be inherited by extensions returned by IExtensible
struct IExtension {
    /// Get the extension's UID
    virtual UID getExtensionID() = 0;
};

/// A class which should be inherited by classes which want to be extensible without breaking the ABI
struct IExtensible {
    /// Try to get an extension by its UID
    /// @return A pointer to the extension or nullptr if it's not supported
    virtual IExtension const* getExtension(UID id) const { return nullptr; }

    /// Query an extension by its type
    /// Don't call directly, use global queryExtension() instead
    /// @typeparam ExtensionT The extension type, must derive from IExtension
    template <class ExtensionT>
    ExtensionT* _queryExtension() const
    {
        static_assert(std::is_base_of<IExtension, ExtensionT>::value, "queryExtension parameter must inherit from IExtension");
        return static_cast<ExtensionT*>(getExtension(ExtensionT::ExtensionIID));
    }

    /// Query an extension by its type
    /// Don't call directly, use global queryExtension() instead
    /// @typeparam ExtensionT The extension type, must derive from IExtension
    template <class ExtensionT>
    ExtensionT* _queryExtension()
    {
        static_assert(std::is_base_of<IExtension, ExtensionT>::value, "queryExtension parameter must inherit from IExtension");
        return const_cast<ExtensionT*>(static_cast<ExtensionT const*>(getExtension(ExtensionT::ExtensionIID)));
    }
};

/// Query an extension by its type
/// @typeparam ExtensionT The extension type, must derive from IExtension
template <class ExtensionT>
ExtensionT* queryExtension(IExtensible* extensible)
{
    return extensible->_queryExtension<ExtensionT>();
}

/// Query an extension by its type
/// @typeparam ExtensionT The extension type, must derive from IExtension
template <class ExtensionT>
ExtensionT* queryExtension(const IExtensible* extensible)
{
    return extensible->_queryExtension<ExtensionT>();
}

/// Query an extension by its type
/// @typeparam ExtensionT The extension type, must derive from IExtension
template <class ExtensionT>
ExtensionT* queryExtension(IExtensible& extensible)
{
    return extensible._queryExtension<ExtensionT>();
}

/// Query an extension by its type
/// @typeparam ExtensionT The extension type, must derive from IExtension
template <class ExtensionT>
ExtensionT* queryExtension(const IExtensible& extensible)
{
    return extensible._queryExtension<ExtensionT>();
}

/// Should always be used in classes inheriting IUIDProvider
#define PROVIDE_UID(uuid)            \
    static constexpr UID IID = uuid; \
    UID getUID() override { return uuid; }

/// An interface providing UIDs
struct IUIDProvider {
    virtual UID getUID() = 0;
};

enum ComponentType {
    Other,
    Network,
    Pool,
};

struct ICore;
struct IComponentList;
struct ILogger;
struct IEarlyConfig;

/// A component interface
struct IComponent : virtual IExtensible, public IUIDProvider {
    /// Get the component's name
    virtual StringView componentName() const = 0;

    /// Get the component's type
    virtual ComponentType componentType() const { return ComponentType::Other; }

    /// Get the component's version
    virtual SemanticVersion componentVersion() const = 0;

    /// Called for every component to load it
    /// Should be used for caching the core interface, registering player/core event handlers
    virtual void onLoad(ICore* c) = 0;

    /// Called when all components have been loaded
    /// Should be used for querying other components, registering their event handlers
    /// Should NOT be used for interacting with other components or firing events
    /// @param components Tcomponentgins list to query
    virtual void onInit(IComponentList* components) { }

    /// Called when all components have been initialised
    /// Should be used for interacting with other components, firing events, or any more complex logic
    virtual void onReady() { }

    /// Called before the components are about to be freed
    /// Should be used for disconnecting
    virtual void onFree(IComponent* component) { }

    /// Fill a configuration object with custom configuration
    virtual void provideConfiguration(ILogger& logger, IEarlyConfig& config) { }

    /// Frees the component data
    virtual void free() = 0;
};

struct IComponentList : public IExtensible {
    /// Query a component by its ID
    /// @param id The UID of the component
    /// @return A pointer to the component or nullptr if not available
    virtual IComponent* queryComponent(UID id) = 0;

    /// Query a component by its type
    /// @typeparam ComponentT The component type, must derive from IComponent
    template <class ComponentT>
    ComponentT* queryComponent()
    {
        static_assert(std::is_base_of<IComponent, ComponentT>::value, "queryComponent parameter must inherit from IComponent");
        return static_cast<ComponentT*>(queryComponent(ComponentT::IID));
    }
};
