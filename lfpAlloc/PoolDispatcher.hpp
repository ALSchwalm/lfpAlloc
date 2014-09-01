#ifndef LF_POOL_DISPATCHER
#define LF_POOL_DISPATCHER

#include <tuple>
#include <cassert>
#include <lfpAlloc/Pool.hpp>

namespace lfpAlloc {
    namespace detail {

        template<std::size_t Num, uint16_t... Ts>
        struct Pools : Pools<Num-1, Power<Num>::value, Ts...>{};

        template<uint16_t... Size>
        struct Pools<log<sizeof(void*)>::value, Size...>{
            using type = std::tuple<Pool<Size, 256*100>...>;
        };
    }

    template<std::size_t MaxPoolPower>
    class PoolDispatcher {
    public:
        void* allocate(std::size_t size) {
            return dispatchAllocate<0>(size);
        }

        void deallocate(void* p, std::size_t size) noexcept {
            dispatchDeallocate<0>(p, size);
        }
    private:
        typename detail::Pools<MaxPoolPower>::type pools;
        static constexpr std::size_t NumPools =
                                std::tuple_size<decltype(pools)>::value;
        static_assert(NumPools > 0, "Invalid number of pool");

        template<std::size_t Index>
        typename std::enable_if<Index < NumPools, void*>::type
        dispatchAllocate(std::size_t requestSize) {
            if (requestSize <= std::get<Index>(pools).size) {
                return std::get<Index>(pools).allocate();
            }
            else {
                return dispatchAllocate<Index+1>(requestSize);
            }
        }

        template<std::size_t Index>
        typename std::enable_if< !(Index < NumPools), void*>::type
        dispatchAllocate(std::size_t requestSize) {
            assert(false && "Invalid allocation size.");
            return nullptr;
        }

        template<std::size_t Index>
        typename std::enable_if<Index < NumPools>::type
        dispatchDeallocate(void* p, std::size_t requestSize) noexcept {
            if (requestSize <= std::get<Index>(pools).size) {
                std::get<Index>(pools).deallocate(p);
            }
            else {
                dispatchDeallocate<Index+1>(p, requestSize);
            }
        }

        template<std::size_t Index>
        typename std::enable_if<!(Index < NumPools)>::type
        dispatchDeallocate(void* p, std::size_t requestSize) noexcept {
            assert(false && "Invalid deallocation size.");
        }
    };
}

#endif
