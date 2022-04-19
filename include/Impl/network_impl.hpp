/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *  
 *  The original code is copyright c 2022, open.mp team and contributors.
 */

#pragma once

#include "../network.hpp"
#include "events_impl.hpp"

/* Implementation, NOT to be passed around */

namespace Impl {

struct Network : public INetwork, public NoCopy {
    DefaultEventDispatcher<NetworkEventHandler> networkEventDispatcher;
    DefaultEventDispatcher<NetworkInEventHandler> inEventDispatcher;
    DefaultIndexedEventDispatcher<SingleNetworkInEventHandler> rpcInEventDispatcher;
    DefaultIndexedEventDispatcher<SingleNetworkInEventHandler> packetInEventDispatcher;
    DefaultEventDispatcher<NetworkOutEventHandler> outEventDispatcher;
    DefaultIndexedEventDispatcher<SingleNetworkOutEventHandler> rpcOutEventDispatcher;
    DefaultIndexedEventDispatcher<SingleNetworkOutEventHandler> packetOutEventDispatcher;

    Network(size_t packetCount, size_t rpcCount)
        : rpcInEventDispatcher(rpcCount)
        , packetInEventDispatcher(packetCount)
        , rpcOutEventDispatcher(rpcCount)
        , packetOutEventDispatcher(packetCount)
    {
    }

    IEventDispatcher<NetworkEventHandler>& getEventDispatcher() override
    {
        return networkEventDispatcher;
    }

    IEventDispatcher<NetworkInEventHandler>& getInEventDispatcher() override
    {
        return inEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInEventHandler>& getPerRPCInEventDispatcher() override
    {
        return rpcInEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInEventHandler>& getPerPacketInEventDispatcher() override
    {
        return packetInEventDispatcher;
    }

    IEventDispatcher<NetworkOutEventHandler>& getOutEventDispatcher() override
    {
        return outEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkOutEventHandler>& getPerRPCOutEventDispatcher() override
    {
        return rpcOutEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkOutEventHandler>& getPerPacketOutEventDispatcher() override
    {
        return packetOutEventDispatcher;
    }
};

}
