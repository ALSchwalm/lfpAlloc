#ifndef LF_POOL_DISPATCHER
#define LF_POOL_DISPATCHER

#include <cstdint>
#include <tuple>
#include <lfpAlloc/Pool.hpp>

namespace lfpAlloc {

    template<std::size_t NumPools>
    class PoolDispatcher {
    public:
        PoolDispatcher() {
            for (std::size_t s=0; s < NumPools; ++s) {
                pools[s] = Pool(s+1);
            }
        }

        void* allocate(std::size_t size) {
            return pools[size].allocate();
        }

        void deallocate(void* p, std::size_t size) noexcept {
            pools[size].deallocate(p);
        }
    private:
        Pool pools[NumPools];
    };
}

#endif
