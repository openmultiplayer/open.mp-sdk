#pragma once

#include <sdk.hpp>

/// The style of the dialog: https://open.mp/docs/scripting/resources/dialogstyles
enum DialogStyle {
    DialogStyle_MSGBOX = 0,
    DialogStyle_INPUT,
    DialogStyle_LIST,
    DialogStyle_PASSWORD,
    DialogStyle_TABLIST,
    DialogStyle_TABLIST_HEADERS
};

enum DialogResponse {
    DialogResponse_Right = 0,
    DialogResponse_Left
};

static const UID DialogData_UID = UID(0xbc03376aa3591a11);
struct IPlayerDialogData : public IPlayerData {
    PROVIDE_UID(DialogData_UID);

    /// Show a dialog to player
    virtual void show(IPlayer& player, uint16_t id, DialogStyle style, StringView caption, StringView info, StringView button1, StringView button2) = 0;

    /// Get player's active dialog
    virtual uint16_t getActiveID() const = 0;
};

struct PlayerDialogEventHandler {
    virtual void onDialogResponse(IPlayer& player, uint16_t dialogId, DialogResponse response, uint16_t listItem, StringView inputText) { }
};

static const UID DialogsComponent_UID = UID(0x44a111350d611dde);
struct IDialogsComponent : public IComponent {
    PROVIDE_UID(DialogsComponent_UID);

    /// Access to event dispatchers for other components to add handlers to
    virtual IEventDispatcher<PlayerDialogEventHandler>& getEventDispatcher() = 0;
};
