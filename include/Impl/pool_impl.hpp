#pragma once

#include "../pool.hpp"
#include "events_impl.hpp"

/* Implementation, NOT to be passed around */

namespace Impl {

template <class T, size_t Count>
struct ScopedPoolReleaseLock {
    IPool<T, Count>& pool;
    int index;
    T& entry;

    ScopedPoolReleaseLock(IPool<T, Count>& pool, int index)
        : pool(pool)
        , index(index)
        , entry(pool.get(index))
    {
        pool.lock(index);
    }

    ScopedPoolReleaseLock(IPool<T, Count>& pool, const IIDProvider& provider)
        : ScopedPoolReleaseLock(pool, provider.getID())
    {
    }

    ~ScopedPoolReleaseLock()
    {
        pool.unlock(index);
    }
};

template <typename T, size_t Size>
struct UniqueIDArray : public NoCopy {
    int findFreeIndex(int from) const
    {
        for (int i = from; i < Size; ++i) {
            if (!valid_[i]) {
                return i;
            }
        }
        return -1;
    }

    void add(int index)
    {
        assert(index < Size);
        valid_.set(index);
    }

    void add(int index, T& data)
    {
        assert(index < Size);
        valid_.set(index);
        entries_.insert(&data);
    }

    /// Attempt to remove data for element at index and return the next iterator in the entries list
    void remove(int index, T& data)
    {
        valid_.reset(index);
        entries_.erase(&data);
    }

    void clear()
    {
        valid_.reset();
        entries_.clear();
    }

    bool valid(int index) const
    {
        if (index >= Size) {
            return false;
        }
        return valid_.test(index);
    }

    const FlatPtrHashSet<T>& entries()
    {
        return entries_;
    }

private:
    StaticBitset<Size> valid_;
    FlatPtrHashSet<T> entries_;
};

template <typename T>
struct UniqueEntryArray : public NoCopy {
    void add(T& data)
    {
        entries_.insert(&data);
    }

    /// Attempt to remove data for element at index and return the next iterator in the entries list
    void remove(T& data)
    {
        entries_.erase(&data);
    }

    void clear()
    {
        entries_.clear();
    }

    const FlatPtrHashSet<T>& entries() const
    {
        return entries_;
    }

private:
    FlatPtrHashSet<T> entries_;
};

struct PoolIDProvider {
    int poolID;
};

template <typename Type, typename Iface, size_t Count>
struct StaticPoolStorageBase : public NoCopy {
    static const size_t Capacity = Count;
    using Interface = Iface;

    template <class... Args>
    Type* emplace(Args&&... args)
    {
        int freeIdx = findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int pid = claimHint(freeIdx, std::forward<Args>(args)...);
        if (pid == -1) {
            // No free index
            return nullptr;
        }

        return &get(pid);
    }

    int findFreeIndex(int from = 0)
    {
        return allocated_.findFreeIndex(from);
    }

    template <class... Args>
    int claim(Args&&... args)
    {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            new (getPtr(freeIdx)) Type(std::forward<Args>(args)...);
            allocated_.add(freeIdx, *getPtr(freeIdx));
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                getPtr(freeIdx)->poolID = freeIdx;
            }
            eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryCreated, *getPtr(freeIdx));
        }
        return freeIdx;
    }

    template <class... Args>
    int claimHint(int hint, Args&&... args)
    {
        assert(hint < Count);
        if (!valid(hint)) {
            new (getPtr(hint)) Type(std::forward<Args>(args)...);
            allocated_.add(hint, *getPtr(hint));
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                getPtr(hint)->poolID = hint;
            }
            eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryCreated, *getPtr(hint));
            return hint;
        } else {
            return claim(std::forward<Args>(args)...);
        }
    }

    void claimUnusable(int index)
    {
        allocated_.add(index);
    }

    bool valid(int index) const
    {
        return allocated_.valid(index);
    }

    Type& get(int index)
    {
        assert(index < Count);
        return *getPtr(index);
    }

    void remove(int index)
    {
        assert(index < Count);
        allocated_.remove(index, *getPtr(index));
        eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *getPtr(index));
        getPtr(index)->~Type();
    }

    ~StaticPoolStorageBase()
    {
        // Placement destructor.
        for (Interface* const ptr : allocated_.entries()) {
            eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *ptr);
            static_cast<Type*>(ptr)->~Type();
        }
    }

    /// Get the raw entries list
    /// Don't use this for looping through entries. Use the custom iterators instead.
    const FlatPtrHashSet<Interface>& _entries()
    {
        return allocated_.entries();
    }

    /// Get the raw entries list
    /// Don't use this for looping through entries. Use the custom iterators instead.
    const FlatPtrHashSet<Interface>& _entries() const
    {
        return allocated_.entries();
    }

    DefaultEventDispatcher<PoolEventHandler<Interface>>& getEventDispatcher()
    {
        return eventDispatcher_;
    }

protected:
    inline Type* getPtr(int index)
    {
        return reinterpret_cast<Type*>(&pool_[index * CEILDIV(sizeof(Type), alignof(Type)) * alignof(Type)]);
    }

    char pool_[Count * sizeof(Type)];
    UniqueIDArray<Interface, Count> allocated_;
    /// Implementation of the pool event dispatcher
    DefaultEventDispatcher<PoolEventHandler<Interface>> eventDispatcher_;
};

template <typename Type, typename Iface, size_t Count>
struct DynamicPoolStorageBase : public NoCopy {
    static const size_t Capacity = Count;
    using Interface = Iface;

    template <class... Args>
    Type* emplace(Args&&... args)
    {
        int freeIdx = findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int pid = claimHint(freeIdx, std::forward<Args>(args)...);
        if (pid == -1) {
            // No free index
            return nullptr;
        }

        return &get(pid);
    }

    DynamicPoolStorageBase()
        : pool_ { nullptr }
    {
    }

    ~DynamicPoolStorageBase()
    {
        for (Interface* const ptr : allocated_.entries()) {
            eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *ptr);
            delete static_cast<Type*>(ptr);
        }
    }

    int findFreeIndex(int from = 0)
    {
        for (int i = from; i < Count; ++i) {
            if (pool_[i] == nullptr) {
                return i;
            }
        }
        return -1;
    }

    template <class... Args>
    int claim(Args&&... args)
    {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            pool_[freeIdx] = new Type(std::forward<Args>(args)...);
            allocated_.add(*pool_[freeIdx]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                pool_[freeIdx]->poolID = freeIdx;
            }
            eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryCreated, *pool_[freeIdx]);
        }
        return freeIdx;
    }

    template <class... Args>
    int claimHint(int hint, Args&&... args)
    {
        assert(hint < Count);
        if (!valid(hint)) {
            pool_[hint] = new Type(std::forward<Args>(args)...);
            allocated_.add(*pool_[hint]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                pool_[hint]->poolID = hint;
            }
            eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryCreated, *pool_[hint]);
            return hint;
        } else {
            return claim(std::forward<Args>(args)...);
        }
    }

    void claimUnusable(int index)
    {
        pool_[index] = reinterpret_cast<Type*>(0x01);
    }

    bool valid(int index) const
    {
        if (index >= Count) {
            return false;
        }
        return pool_[index] != nullptr;
    }

    Type& get(int index)
    {
        assert(index < Count);
        return *pool_[index];
    }

    void remove(int index)
    {
        assert(index < Count);
        allocated_.remove(*pool_[index]);
        eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *pool_[index]);
        delete pool_[index];
        pool_[index] = nullptr;
    }

    /// Get the raw entries list
    /// Don't use this for looping through entries. Use the custom iterators instead.
    const FlatPtrHashSet<Interface>& _entries() const
    {
        return allocated_.entries();
    }

    DefaultEventDispatcher<PoolEventHandler<Interface>>& getEventDispatcher()
    {
        return eventDispatcher_;
    }

protected:
    StaticArray<Type*, Count> pool_;
    UniqueEntryArray<Interface> allocated_;
    /// Implementation of the pool event dispatcher
    DefaultEventDispatcher<PoolEventHandler<Interface>> eventDispatcher_;
};

template <class PoolBase>
struct ImmediatePoolStorageLifetimeBase final : public PoolBase {
    inline void release(int index)
    {
        PoolBase::remove(index);
    }

    /// Get the entries list
    const FlatPtrHashSet<typename PoolBase::Interface>& entries()
    {
        return PoolBase::allocated_.entries();
    }

    /// Get the entries list
    const FlatPtrHashSet<typename PoolBase::Interface>& entries() const
    {
        return PoolBase::allocated_.entries();
    }
};

template <class PoolBase, typename RefCountType = uint8_t>
struct MarkedPoolStorageLifetimeBase final : public PoolBase {
    using Iterator = MarkedPoolIterator<typename PoolBase::Interface, MarkedPoolStorageLifetimeBase<PoolBase, RefCountType>>;

    /// Return the begin iterator
    inline Iterator begin()
    {
        return Iterator(*this, PoolBase::_entries(), PoolBase::_entries().begin());
    }

    /// Return the end iterator
    inline Iterator end()
    {
        return Iterator(*this, PoolBase::_entries(), PoolBase::_entries().end());
    }

    MarkedPoolStorageLifetimeBase()
        : refs_ {}
    {
    }

    void lock(int index)
    {
        // Increase number of lock refs
        ++refs_[index];
        assert(refs_[index] < std::numeric_limits<RefCountType>::max());
    }

    bool unlock(int index)
    {
        assert(refs_[index] > 0);
        // If marked for deletion on unlock, release
        if (--refs_[index] == 0 && deleted_.test(index)) {
            release(index, true);
            return true;
        }
        return false;
    }

    void release(int index, bool force)
    {
        assert(index < PoolBase::Capacity);
        // If locked, mark for deletion on unlock
        if (refs_[index] > 0) {
            deleted_.set(index);
        } else { // If not locked, immediately delete
            deleted_.reset(index);
            PoolBase::remove(index);
        }
    }

private:
    /// List signifying whether an entry is marked for deletion
    StaticBitset<PoolBase::Capacity> deleted_;
    /// List signifying the number of references held for the entry; if 0 and marked for deletion, it's deleted
    StaticArray<RefCountType, PoolBase::Capacity> refs_;
};

/// Pool storage which doesn't mark entries for release but immediately releases
/// Allocates contents statically
template <typename Type, typename Interface, size_t Count>
using PoolStorage = ImmediatePoolStorageLifetimeBase<StaticPoolStorageBase<Type, Interface, Count>>;

/// Pool storage which doesn't mark entries for release but immediately releases
/// Allocates contents dynamically
template <typename Type, typename Interface, size_t Count>
using DynamicPoolStorage = ImmediatePoolStorageLifetimeBase<DynamicPoolStorageBase<Type, Interface, Count>>;

/// Pool storage which marks entries for release if locked
/// Allocates contents statically
template <typename Type, typename Interface, size_t Count, typename RefCountType = uint8_t>
using MarkedPoolStorage = MarkedPoolStorageLifetimeBase<StaticPoolStorageBase<Type, Interface, Count>, RefCountType>;

/// Pool storage which marks entries for release if locked
/// Allocates contents dynamically
template <typename Type, typename Interface, size_t Count, typename RefCountType = uint8_t>
using MarkedDynamicPoolStorage = MarkedPoolStorageLifetimeBase<DynamicPoolStorageBase<Type, Interface, Count>, RefCountType>;

}
