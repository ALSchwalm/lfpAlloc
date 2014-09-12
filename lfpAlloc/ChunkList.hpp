#ifndef LF_POOL_ALLOC_CHUNK_LIST
#define LF_POOL_ALLOC_CHUNK_LIST

#include <cstdint>
#include <atomic>

#ifndef LFP_ALLOW_BLOCKING
static_assert(ATOMIC_POINTER_LOCK_FREE==2, "Atomic pointer is not lock-free.");
#endif

namespace lfpAlloc {

    template<std::size_t Size>
    struct Cell {
        Cell* next_ = this+1;
        uint8_t val[Size];
    };

    template<std::size_t Size, std::size_t AllocationsPerChunk>
    struct Chunk {
        Chunk() noexcept {
            auto& last = memBlock_[AllocationsPerChunk-1];
            last.next_ = nullptr;
        }
        Cell<Size> memBlock_[AllocationsPerChunk];
        Chunk* next_ = nullptr;
    };

    template<std::size_t Size>
    class ChunkList {
        
    };
}

#endif
