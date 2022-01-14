#pragma once

#include "../network.hpp"
#include "events_impl.hpp"

/* Implementation, NOT to be passed around */

namespace Impl {

struct Network : public INetwork, public NoCopy {
    DefaultEventDispatcher<NetworkEventHandler> networkEventDispatcher;
    DefaultEventDispatcher<NetworkInOutEventHandler> inOutEventDispatcher;
    DefaultIndexedEventDispatcher<SingleNetworkInOutEventHandler> rpcInOutEventDispatcher;
    DefaultIndexedEventDispatcher<SingleNetworkInOutEventHandler> packetInOutEventDispatcher;

    Network(size_t packetCount, size_t rpcCount)
        : rpcInOutEventDispatcher(rpcCount)
        , packetInOutEventDispatcher(packetCount)
    {
    }

    IEventDispatcher<NetworkEventHandler>& getEventDispatcher() override
    {
        return networkEventDispatcher;
    }

    IEventDispatcher<NetworkInOutEventHandler>& getInOutEventDispatcher() override
    {
        return inOutEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInOutEventHandler>& getPerRPCInOutEventDispatcher() override
    {
        return rpcInOutEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInOutEventHandler>& getPerPacketInOutEventDispatcher() override
    {
        return packetInOutEventDispatcher;
    }
};

}