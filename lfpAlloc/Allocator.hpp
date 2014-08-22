#ifndef LF_POOL_ALLOCATOR
#define LF_POOL_ALLOCATOR

#include <memory>
#include <lfpAlloc/PoolDispatcher.hpp>

namespace lfpAlloc {
    template<typename T, std::size_t MaxChunkSize=8>
    class lfpAllocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const pointer;
        using reference = T&;
        using const_reference = const reference;

        template<typename U>
        struct rebind {
            typedef lfpAllocator<U, MaxChunkSize> other;
        };

        lfpAllocator() : dispatcher(new PoolDispatcher<T, MaxChunkSize>) {}

        lfpAllocator(const lfpAllocator& other) noexcept :
            dispatcher(other.dispatcher) {}

        lfpAllocator& operator=(const lfpAllocator& other) noexcept {
            dispatcher.reset(other.dispatcher.get());
            return *this;
        }

        T* allocate(std::size_t size) {
            return dispatcher->allocate(size);
        }

        void deallocate(T* p, std::size_t size) noexcept {
            dispatcher->deallocate(p, size);
        }

        // Should not be required, but allocator_traits is not complete in
        // gcc 4.9.1
        template<typename U>
        void destroy(U* p) {
            p->~U();
        }

        template<typename U, typename... Args >
        void construct( U* p, Args&&... args ) {
            new (p) U(std::forward<Args>(args)...);
        }

        template<typename Ty, typename U, std::size_t N, std::size_t M>
        friend bool operator==(const lfpAllocator<Ty, N>& left,
                               const lfpAllocator<U, M>& right) noexcept;

    private:
        std::shared_ptr<PoolDispatcher<T, MaxChunkSize>> dispatcher;
    };

    template<typename T, typename U, std::size_t N, std::size_t M>
    inline bool operator==(const lfpAllocator<T, N>& left,
                           const lfpAllocator<U, M>& right) noexcept {
        return left.dispatcher.get() == right.dispatcher.get();
    }

    template<typename T, typename U, std::size_t N, std::size_t M>
    inline bool operator==(const lfpAllocator<T, N>& left,
                           const lfpAllocator<T, M>& right) noexcept {
        return !(left == right);
    }
}

#endif
