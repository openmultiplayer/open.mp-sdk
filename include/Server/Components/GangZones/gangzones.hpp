#pragma once

#include <anim.hpp>
#include <component.hpp>
#include <player.hpp>
#include <types.hpp>
#include <values.hpp>

struct GangZonePos {
    Vector2 min = { 0.0f, 0.0f };
    Vector2 max = { 0.0f, 0.0f };
};

/// Gangzone interace
struct IGangZone : public IIDProvider {
    /// Check if a gangzone is shown for player
    virtual bool isShownForPlayer(const IPlayer& player) const = 0;

    /// Show a gangzone for player
    virtual void showForPlayer(IPlayer& player, const Colour& colour) = 0;

    /// Hide a gangzone for player
    virtual void hideForPlayer(IPlayer& player) = 0;

    /// Flashing a gangzone for player
    virtual void flashForPlayer(IPlayer& player, const Colour& colour) = 0;

    /// Stop flashing a gangzone for player
    virtual void stopFlashForPlayer(IPlayer& player) = 0;

    /// Get position of gangzone. Returns a structure of vec2 min and vec2 max coordinates
    virtual GangZonePos getPosition() const = 0;

    /// Set position of gangzone. Takes a structure of vec2 min and vec2 max coordinates
    virtual void setPosition(const GangZonePos& position) = 0;

    /// get a list of players inside of gangzone
    virtual FlatHashSet<IPlayer*>& getPlayersInside() = 0;
};

struct GangZoneEventHandler {
    virtual void OnPlayerEnterGangZone(IPlayer& player, IGangZone& zone) { }
    virtual void OnPlayerLeaveGangZone(IPlayer& player, IGangZone& zone) { }
};

static const UID GangZoneComponent_UID = UID(0xb3351d11ee8d8056);

struct IGangZonesComponent : public IPoolComponent<IGangZone /*, GANG_ZONE_POOL_SIZE*/> {
    PROVIDE_UID(GangZoneComponent_UID);

    /// Get the event dispatcher
    virtual IEventDispatcher<GangZoneEventHandler>& getEventDispatcher() = 0;

    /// Create a gang zone
    virtual IGangZone* create(GangZonePos pos) = 0;
};
