/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *  
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include <core.hpp>

constexpr int NUM_AMX_FUNCS = 44;

struct PawnEventHandler {
    virtual void onAmxLoad(void* amx) = 0;
    virtual void onAmxUnload(void* amx) = 0;
};

static const UID PawnComponent_UID = UID(0x78906cd9f19c36a6);
struct IPawnComponent : public IComponent {
    PROVIDE_UID(PawnComponent_UID);

    /// Get the ConsoleEventHandler event dispatcher
    virtual IEventDispatcher<PawnEventHandler>& getEventDispatcher() = 0;

    virtual const StaticArray<void*, NUM_AMX_FUNCS>& getAmxFunctions() const = 0;
};
