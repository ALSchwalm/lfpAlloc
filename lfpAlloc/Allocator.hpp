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

        lfpAllocator(const lfpAllocator& other) : dispatcher(other.dispatcher){}
        lfpAllocator& operator=(const lfpAllocator& other) {
            dispatcher.reset(other.dispatcher.get());
            return *this;
        }

        T* allocate(std::size_t size) {
            return dispatcher->allocate(size);
        }

        void deallocate(T* p, std::size_t size) {
            dispatcher->deallocate(p, size);
        }

        template<typename U>
        void destroy(U* p) {
            p->~U();
        }

        template<typename U, typename... Args >
        void construct( U* p, Args&&... args ){
            ::new((void*)p) U(std::forward<Args>(args)...);
        }

        bool operator==(const lfpAllocator& other) const {
            return dispatcher.get() == other.dispatcher.get();
        }

    private:
        std::shared_ptr<PoolDispatcher<T, MaxChunkSize>> dispatcher;
    };
}

#endif
