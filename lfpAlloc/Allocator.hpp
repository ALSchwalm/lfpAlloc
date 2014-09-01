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
            dispatcher(new PoolDispatcher<MaxPoolPower>[std::thread::hardware_concurrency()],
                       [](PoolDispatcher<MaxPoolPower>*p) { delete[] p; }) {}

        lfpAllocator(const lfpAllocator& other) noexcept :
            dispatcher(other.dispatcher) {}

        template<typename U>
        lfpAllocator(const lfpAllocator<U, MaxPoolPower>& other) noexcept :
            dispatcher(other.getDispatcher()) {}

        lfpAllocator& operator=(const lfpAllocator& other) noexcept {
            dispatcher.reset(other.dispatcher.get());
            return *this;
        }

        T* allocate(std::size_t count) {
            if (detail::hashedID == -1) {
                detail::hashedID = detail::currendThreadHashedID();
            }

            if (sizeof(T)*count <= detail::Power<MaxPoolPower>::value) {
                return reinterpret_cast<T*>((dispatcher.get()+detail::hashedID)->allocate(sizeof(T)*count));
            } else {
                return new T[count];
            }
        }

        void deallocate(T* p, std::size_t count) noexcept {
            if (sizeof(T)*count <= detail::Power<MaxPoolPower>::value) {
                (dispatcher.get()+detail::hashedID)->deallocate(p, sizeof(T)*count);
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

        const std::shared_ptr<PoolDispatcher<MaxPoolPower>>& getDispatcher() const {
            return dispatcher;
        }

        template<typename Ty, typename U, std::size_t N, std::size_t M>
        friend bool operator==(const lfpAllocator<Ty, N>& left,
                               const lfpAllocator<U, M>& right) noexcept;

    private:
        std::shared_ptr<PoolDispatcher<MaxPoolPower>> dispatcher;
    };

    template<typename T, typename U, std::size_t N, std::size_t M>
    inline bool operator==(const lfpAllocator<T, N>& left,
                           const lfpAllocator<U, M>& right) noexcept {
        return left.dispatcher.get() == right.dispatcher.get();
    }

    template<typename T, typename U, std::size_t N, std::size_t M>
    inline bool operator!=(const lfpAllocator<T, N>& left,
                           const lfpAllocator<U, M>& right) noexcept {
        return !(left == right);
    }
}

#endif
