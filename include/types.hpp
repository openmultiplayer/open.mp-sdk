#pragma once

#include "exports.hpp"
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/string_view.h>
#include <absl/types/optional.h>
#include <absl/types/span.h>
#include <absl/types/variant.h>
#include <array>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <queue>
#include <stack>
#include <string>
#include <utility>
#include <vector>

/* Fix Ubuntu 18.04 build - possibly remove when EOL depending on which
 * other distributions we might want to support (18.04 uses glibc 2.27)
 * (see: https://sourceware.org/bugzilla/show_bug.cgi?id=19239%22)
 */
#ifdef major
#undef major
#endif
#ifdef minor
#undef minor
#endif

namespace absl_omp = absl::lts_20210324;

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef uint64_t UUID;
typedef std::chrono::steady_clock Time;
typedef std::chrono::steady_clock::time_point TimePoint;
typedef std::chrono::system_clock WorldTime;
typedef std::chrono::system_clock::time_point WorldTimePoint;
typedef std::chrono::nanoseconds Nanoseconds;
typedef std::chrono::microseconds Microseconds;
typedef std::chrono::milliseconds Milliseconds;
typedef std::chrono::seconds Seconds;
typedef std::chrono::minutes Minutes;
typedef std::chrono::hours Hours;
typedef std::chrono::duration<float> RealSeconds;
using std::chrono::duration_cast;

template <typename... Args>
using Variant = absl_omp::variant<Args...>;

template <typename T>
using Optional = absl_omp::optional<T>;

template <typename T>
using Span = absl_omp::Span<T>;

/// Don't pass String around the SDK, only StringView
using String = std::string;
using StringView = absl_omp::string_view;

template <typename T>
using FlatHashSet = absl_omp::flat_hash_set<T>;
template <typename K, typename V>
using FlatHashMap = absl_omp::flat_hash_map<K, V>;

template <typename T>
using FlatRefHashSet = FlatHashSet<std::reference_wrapper<T>>;
template <typename T>
using FlatPtrHashSet = FlatHashSet<T*>;

template <typename T, size_t Size>
using StaticArray = std::array<T, Size>;

template <typename T>
using DynamicArray = std::vector<T>;

template <typename T>
using Queue = std::queue<T>;

template <typename T>
using Stack = std::stack<T>;

template <size_t Size>
using StaticBitset = std::bitset<Size>;

template <typename First, typename Second>
using Pair = std::pair<First, Second>;

template <typename T, typename... Args>
inline auto variant_get(Args&&... args) -> decltype(absl_omp::get<T>(std::forward<Args>(args)...))
{
    return absl_omp::get<T>(std::forward<Args>(args)...);
}

struct NoCopy {
    NoCopy() = default;

    NoCopy(const NoCopy& other) = delete;
    NoCopy(NoCopy&& other) = delete;

    NoCopy& operator=(const NoCopy& other) = delete;
    NoCopy& operator=(NoCopy&& other) = delete;
};

struct Colour {
    union {
        struct {
            uint8_t r, g, b, a;
        };
    };

    Colour() = default;

    Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF)
        : r(r)
        , g(g)
        , b(b)
        , a(a)
    {
    }

    uint32_t RGBA() const
    {
        return ((r << 24) & 0xFF000000) | ((g << 16) & 0x00FF0000) | ((b << 8) & 0x0000FF00) | (a & 0x000000FF);
    }

    uint32_t ARGB() const
    {
        return ((a << 24) & 0xFF000000) | ((r << 16) & 0x00FF0000) | ((g << 8) & 0x0000FF00) | (b & 0x000000FF);
    }

    uint32_t ABGR() const
    {
        return ((a << 24) & 0xFF000000) | ((b << 16) & 0x00FF0000) | ((g << 8) & 0x0000FF00) | (r & 0x000000FF);
    }

    static Colour FromRGBA(uint32_t from)
    {
        Colour c;
        c.r = (from & 0xFF000000) >> 24;
        c.g = (from & 0x00FF0000) >> 16;
        c.b = (from & 0x0000FF00) >> 8;
        c.a = (from & 0x000000FF);
        return c;
    }

    static Colour FromARGB(uint32_t from)
    {
        Colour c;
        c.a = (from & 0xFF000000) >> 24;
        c.r = (from & 0x00FF0000) >> 16;
        c.g = (from & 0x0000FF00) >> 8;
        c.b = (from & 0x000000FF);
        return c;
    }

    static Colour White()
    {
        return Colour::FromRGBA(0xFFFFFFFF);
    }

    static Colour Black()
    {
        return Colour::FromRGBA(0x000000FF);
    }

    static Colour None()
    {
        return Colour::FromRGBA(0);
    }

    static Colour Yellow()
    {
        return Colour::FromRGBA(0xFFFF00FF);
    }

    static Colour Cyan()
    {
        return Colour::FromRGBA(0x00FFFFFF);
    }
};

/// SemVer
struct SemanticVersion {
    uint8_t major; ///< MAJOR version when you make incompatible API changes
    uint8_t minor; ///< MINOR version when you add functionality in a backwards compatible manner
    uint8_t patch; ///< PATCH version when you make backwards compatible bug fixes
    uint16_t prerel; ///< PRE-RELEASE version

    SemanticVersion(uint8_t major, uint8_t minor, uint8_t patch, uint8_t prerel = 0)
        : major(major)
        , minor(minor)
        , patch(patch)
    {
    }

    int compare(const SemanticVersion& other, bool cmpPreRel) const
    {
        if (major != other.major) {
            return major - other.major;
        }

        if (minor != other.minor) {
            return minor - other.minor;
        }

        if (patch != other.patch) {
            return patch - other.patch;
        }

        if (cmpPreRel && prerel != other.prerel) {
            return prerel - other.prerel;
        }

        return 0;
    }

    bool operator==(const SemanticVersion& other) const
    {
        return compare(other, false) == 0;
    }

    bool operator!=(const SemanticVersion& other) const
    {
        return !(*this == other);
    }

    bool operator>(const SemanticVersion& other) const
    {
        return compare(other, false) > 0;
    }

    bool operator<(const SemanticVersion& other) const
    {
        return compare(other, false) < 0;
    }

    bool operator>=(const SemanticVersion& other) const
    {
        return compare(other, false) >= 0;
    }

    bool operator<=(const SemanticVersion& other) const
    {
        return compare(other, false) <= 0;
    }
};

/// An ABI-stable, C-compatible string that allows for specifying a static allocation of a size before falling back to dynamic allocation
template <size_t Size>
struct StaticString {
    /// The actualy usable size - this accounts for the trailing 0
    constexpr static size_t UsableStaticSize = Size - 1;

    /// Empty string constructor
    StaticString()
        : lenDynamic(0)
    {
        static_assert(Size > 0);
        staticStorage[0] = 0;
    }

    /// StringView copy constructor
    StaticString(StringView string)
    {
        initCopy(string.data(), string.length());
    }

    /// StringView copy assignment
    StaticString<Size>& operator=(StringView string)
    {
        clear();
        initCopy(string.data(), string.length());

        return *this;
    }

    /// Copy constructor
    StaticString(const StaticString<Size>& other)
    {
        initCopy(other.data(), other.length());
    }

    /// Copy assignment
    StaticString<Size>& operator=(const StaticString<Size>& other)
    {
        clear();
        initCopy(other.data(), other.length());

        return *this;
    }

    /// Move constructor
    StaticString(StaticString<Size>&& other)
    {
        initMove(other.data(), other.length());
        other.lenDynamic = 0;
    }

    /// Move assignment
    StaticString<Size>& operator=(StaticString<Size>&& other)
    {
        clear();
        initMove(other.data(), other.length());
        other.lenDynamic = 0;

        return *this;
    }

    /// Destructor
    ~StaticString()
    {
        clear();
    }

    /// Reserve a given length for outside filling
    void reserve(size_t len)
    {
        clear();
        initReserve(len);
    }

    /// Get the data
    constexpr char* data()
    {
        return dynamic() ? dynamicStorage : staticStorage;
    }

    /// Get the data
    constexpr const char* data() const
    {
        return dynamic() ? dynamicStorage : staticStorage;
    }

    /// Get whether the string is dynamically allocated
    constexpr bool dynamic() const
    {
        return lenDynamic & 1;
    }

    /// Get the string's length
    constexpr size_t length() const
    {
        return lenDynamic >> 1;
    }

    /// Clear the string and free any dynamic memory
    void clear()
    {
        if (dynamic()) {
            omp_free(dynamicStorage);
        }
        staticStorage[0] = 0;
        lenDynamic = 0;
    }

    /// Compare the string to another string
    int cmp(const StaticString<Size>& other) const
    {
        return strcmp(data(), other.data());
    }

    /// Return whether the string is equal to another string
    bool operator==(const StaticString<Size>& other) const
    {
        if (length() != other.length()) {
            return false;
        }
        return !strncmp(data(), other.data(), length());
    }

    /// Cast to StringView
    operator StringView() const
    {
        return StringView(data(), length());
    }

private:
    /// Copy data
    void initCopy(const char* data, size_t len)
    {
        const bool isDynamic = len > UsableStaticSize;
        lenDynamic = (len << 1) | isDynamic;
        char* ptr;
        if (isDynamic) {
            dynamicStorage = reinterpret_cast<char*>(omp_malloc(sizeof(char) * (len + 1)));
            ptr = dynamicStorage;
        } else {
            ptr = staticStorage;
        }
        memcpy(ptr, data, len);
        ptr[len] = 0;
    }

    /// Move data
    void initMove(char* data, size_t len)
    {
        const bool isDynamic = len > UsableStaticSize;
        lenDynamic = (len << 1) | isDynamic;
        if (isDynamic) {
            dynamicStorage = data;
        } else {
            memcpy(staticStorage, data, len);
            staticStorage[len] = 0;
        }
    }

    /// Reserve data
    void initReserve(size_t len)
    {
        const bool isDynamic = len > UsableStaticSize;
        lenDynamic = (len << 1) | isDynamic;
        if (isDynamic) {
            dynamicStorage = reinterpret_cast<char*>(omp_malloc(sizeof(char) * (len + 1)));
            dynamicStorage[0] = 0;
        }
    }

    size_t lenDynamic; ///< First bit is 1 if dynamic and 0 if static; the rest are the length
    union {
        char* dynamicStorage; ///< Used when first bit of lenDynamic is 1
        char staticStorage[Size]; ///< Used when first bit of lenDynamic is 0
    };
};

template <typename T, typename U>
inline constexpr auto CEILDIV(T n, U d) -> decltype(n / d)
{
    return (n) ? ((n - (T)1) / d + (decltype(n / d))1) : (decltype(n / d))0;
}
