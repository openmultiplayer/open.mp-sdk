#pragma once

#include "types.hpp"

/* Interfaces, to be passed around */

/// Holds data to pass when applying an animation to a player
struct AnimationData {
    float delta; ///< The speed to play the animation
    bool loop; ///< If set to 1, the animation will loop. If set to 0, the animation will play once
    bool lockX; ///< If set to 0, the player is returned to their old X coordinate once the animation is complete (for animations that move the player such as walking). 1 will not return them to their old position
    bool lockY; ///< Same as above but for the Y axis. Should be kept the same as the previous parameter
    bool freeze; ///< Setting this to 1 will freeze the player at the end of the animation. 0 will not
    uint32_t time; ///< Timer in milliseconds. For a never-ending loop it should be 0
    StaticString<64> lib; ///< The animation library of the animation to apply
    StaticString<64> name; ///< The name of the animation to apply

    AnimationData()
        : delta(4.1f)
        , loop(false)
        , lockX(false)
        , lockY(false)
        , freeze(false)
        , time(0)
    {
    }

    /// Constructor
    AnimationData(float delta, bool loop, bool lockX, bool lockY, bool freeze, uint32_t time, StringView lib, StringView name)
        : delta(delta)
        , loop(loop)
        , lockX(lockX)
        , lockY(lockY)
        , freeze(freeze)
        , time(time)
        , lib(lib)
        , name(name)
    {
    }
};
