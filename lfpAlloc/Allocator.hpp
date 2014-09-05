#ifndef LF_POOL_ALLOCATOR
#define LF_POOL_ALLOCATOR

#include <memory>
#include <lfpAlloc/PoolDispatcher.hpp>
#include <thread>

namespace lfpAlloc {
    namespace detail {
        static std::atomic<char> currentID(0);
        int currendThreadHashedID() {
            return currentID++ % std::thread::hardware_concurrency();;
        }
        static thread_local int hashedID=-1;
    }

    template<typename T, std::size_t MaxPoolPower=8>
    class lfpAllocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const pointer;
        using reference = T&;
        using const_reference = const reference;

        template<typename U>
        struct rebind {
            typedef lfpAllocator<U, MaxPoolPower> other;
        };

        lfpAllocator() :
            dispatcher_(new PoolDispatcher<MaxPoolPower>[std::thread::hardware_concurrency()],
                       [](PoolDispatcher<MaxPoolPower>*p) { delete[] p; }) {}

        template<typename U>
        lfpAllocator(lfpAllocator<U, MaxPoolPower>&& other) noexcept :
            dispatcher_(std::move(other.dispatcher_)) {}

        template<typename U>
        lfpAllocator(const lfpAllocator<U, MaxPoolPower>& other) noexcept :
            dispatcher_(other.dispatcher_) {}

        T* allocate(std::size_t count) {
            if (detail::hashedID == -1) {
                detail::hashedID = detail::currendThreadHashedID();
            }

            if (sizeof(T)*count <= alignof(std::max_align_t)*MaxPoolPower-sizeof(void*)) {
                auto dispatch = dispatcher_.get()+detail::hashedID;
                return reinterpret_cast<T*>(dispatch->allocate(sizeof(T)*count));
            } else {
                return new T[count];
            }
        }

        void deallocate(T* p, std::size_t count) noexcept {
            if (sizeof(T)*count <= alignof(std::max_align_t)*MaxPoolPower-sizeof(void*)) {
                (dispatcher_.get()+detail::hashedID)->deallocate(p, sizeof(T)*count);
            } else {
                delete[] p;
            }
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

        template<typename U, std::size_t M>
        friend class lfpAllocator;
    private:
        std::shared_ptr<PoolDispatcher<MaxPoolPower>> dispatcher_;
    };

    template<typename T, typename U, std::size_t N, std::size_t M>
    inline bool operator==(const lfpAllocator<T, N>& left,
                           const lfpAllocator<U, M>& right) noexcept {
        return left.dispatcher_.get() == right.dispatcher_.get();
    }

    template<typename T, typename U, std::size_t N, std::size_t M>
    inline bool operator!=(const lfpAllocator<T, N>& left,
                           const lfpAllocator<U, M>& right) noexcept {
        return !(left == right);
    }
}

#endif
