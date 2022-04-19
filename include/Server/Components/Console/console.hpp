/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *  
 *  The original code is copyright c 2022, open.mp team and contributors.
 */

#pragma once
#include <core.hpp>
#include <types.hpp>

/// Used for custom handling of messages received by the console
struct ConsoleMessageHandler {
    virtual void handleConsoleMessage(StringView message) = 0;
};

/// The command sender types
enum class ConsoleCommandSender {
    Console,
    Player,
    Custom
};

/// Data attached to the command sender types
struct ConsoleCommandSenderData {
    ConsoleCommandSender sender;
    union {
        IPlayer* player; ///< The player who sent the command, only available with ConsoleCommandSender::Player
        ConsoleMessageHandler* handler; ///< The handler of the command sender, only available with ConsoleCommandSender::Custom
    };

    /// Console sender constructor
    ConsoleCommandSenderData()
        : sender(ConsoleCommandSender::Console)
        , player(nullptr)
    {
    }

    /// Player sender constructor
    ConsoleCommandSenderData(IPlayer& player)
        : sender(ConsoleCommandSender::Player)
        , player(&player)
    {
    }

    /// Custom sender constructor
    ConsoleCommandSenderData(ConsoleMessageHandler& handler)
        : sender(ConsoleCommandSender::Custom)
        , handler(&handler)
    {
    }
};

struct ConsoleEventHandler {
    virtual bool onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender) { return false; }
    virtual void onRconLoginAttempt(IPlayer& player, StringView password, bool success) { }
};

static const UID ConsoleComponent_UID = UID(0xbfa24e49d0c95ee4);
struct IConsoleComponent : public IComponent {
    PROVIDE_UID(ConsoleComponent_UID);

    /// Get the ConsoleEventHandler event dispatcher
    virtual IEventDispatcher<ConsoleEventHandler>& getEventDispatcher() = 0;

    /// Send a console command
    virtual void send(StringView command, const ConsoleCommandSenderData& sender = ConsoleCommandSenderData()) = 0;
    virtual void sendMessage(const ConsoleCommandSenderData& recipient, StringView message) = 0;
};

static const UID PlayerConsoleData_UID = UID(0x9f8d20f2f471cbae);
struct IPlayerConsoleData : public IExtension {
    PROVIDE_EXT_UID(PlayerConsoleData_UID);

    /// Check if player has console access
    virtual bool hasConsoleAccess() const = 0;
};
