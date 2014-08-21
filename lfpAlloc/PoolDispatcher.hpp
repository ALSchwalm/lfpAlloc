#ifndef LF_POOL_DISPATCHER
#define LF_POOL_DISPATCHER

#include <cstdint>
#include <tuple>
#include <lfpAlloc/Pool.hpp>

namespace lfpAlloc {
    namespace detail {
        template<std::size_t Count, std::size_t Val=1>
        struct Power : Power<Count-1, Val*2>{};

        template<std::size_t Val>
        struct Power<0, Val> {
            enum {
                value = Val/2
            };
        };

        template<typename T, std::size_t Num, uint16_t... Ts>
        struct Pools : Pools<T, Num-1, Power<Num>::value, Ts...>{};

        template<typename T, uint16_t... CPA>
        struct Pools<T, 0, CPA...>{
            using type = std::tuple<Pool<T, CPA, 256*100>...>;
        };
    }

    template<typename T, std::size_t MaxChunkSize>
    class PoolDispatcher {
    public:
        T* allocate(std::size_t size) {
            return dispatchAllocate<1>(size);
        }

        void deallocate(T* p, std::size_t size) {
            dispatchDeallocate<1>(p, size);
        }
    private:
        template<std::size_t ChunkSize>
        typename std::enable_if<ChunkSize <= MaxChunkSize, T*>::type
        dispatchAllocate(std::size_t requestSize) {
            if (requestSize <= detail::Power<ChunkSize>::value) {
                return std::get<ChunkSize-1>(pools).allocate();
            }
            else {
                return dispatchAllocate<ChunkSize+1>(requestSize);
            }
        }

        template<std::size_t ChunkSize>
        typename std::enable_if< !(ChunkSize <= MaxChunkSize), T*>::type
        dispatchAllocate(std::size_t requestSize) {
            return new T[requestSize];
        }

        template<std::size_t ChunkSize>
        typename std::enable_if<ChunkSize <= MaxChunkSize>::type
        dispatchDeallocate(T* p, std::size_t requestSize) {
            if (requestSize <= detail::Power<ChunkSize>::value) {
                std::get<ChunkSize-1>(pools).deallocate(p);
            }
            else {
                dispatchDeallocate<ChunkSize+1>(p, requestSize);
            }
        }

        template<std::size_t ChunkSize>
        typename std::enable_if<!(ChunkSize <= MaxChunkSize)>::type
        dispatchDeallocate(T* p, std::size_t requestSize) {
            delete[] p;
        }

        typename detail::Pools<T, MaxChunkSize>::type pools;
    };
}

#endif
