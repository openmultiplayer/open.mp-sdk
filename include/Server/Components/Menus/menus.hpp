#pragma once

#include <component.hpp>
#include <player.hpp>
#include <types.hpp>
#include <values.hpp>

typedef uint8_t MenuRow;
typedef uint8_t MenuColumn;

/// Pickup interace
struct IMenu : public IIDProvider {
    /// Sets given column's header title
    virtual void setColumnHeader(StringView header, MenuColumn column) = 0;

    /// Adds an item to given column
    virtual void addMenuItem(StringView itemText, MenuColumn column) = 0;

    /// Disables a specific row in menu
    virtual void disableMenuRow(MenuRow row) = 0;

    /// Disables menu
    virtual void disableMenu() = 0;

    /// Initialise menu for a player
    virtual void initForPlayer(IPlayer& player) = 0;

    /// Show menu for a player
    virtual void showForPlayer(IPlayer& player) = 0;

    /// Hide menu for a player
    virtual void hideForPlayer(IPlayer& player) = 0;
};

struct MenuEventHandler {
    virtual void onPlayerSelectedMenuRow(IPlayer& player, MenuRow row) { }
    virtual void onPlayerExitedMenu(IPlayer& player) { }
};

static const UniqueID PlayerMenuData_UniqueID = UniqueID(0x01d8e934e9791b99);
struct IPlayerMenuData : public IPlayerData {
    PROVIDE_UniqueID(PlayerMenuData_UniqueID)

    /// Get Player's current menu id
    virtual uint8_t getMenuID() const = 0;

    /// Set Player's current menu id
    virtual void setMenuID(uint8_t id) = 0;
};

static const UniqueID MenusComponent_UniqueID = UniqueID(0x621e219eb97ee0b2);
struct IMenusComponent : public IPoolComponent<IMenu, MENU_POOL_SIZE> {
    PROVIDE_UniqueID(MenusComponent_UniqueID);

    virtual IEventDispatcher<MenuEventHandler>& getEventDispatcher() = 0;

    /// Create a menu
    virtual IMenu* create(StringView title, Vector2 position, uint8_t columns, float col1Width, float col2Width) = 0;
};
