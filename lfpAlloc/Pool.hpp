#ifndef LF_POOL_ALLOC_POOL
#define LF_POOL_ALLOC_POOL

#include <lfpAlloc/Utils.hpp>
#include <lfpAlloc/ChunkList.hpp>

namespace lfpAlloc {
    template<std::size_t Size, std::size_t AllocationsPerChunk>
    class Pool {
    public:
        static constexpr std::size_t CellSize = Size-sizeof(void*);
        using Cell_t = Cell<CellSize>;

        Pool() : head_(nullptr){}

        ~Pool() {
            chunkList_.deallocateChain(head_);
        }

        void* allocate(){
            // Head loaded from head_
            Cell_t* currentHead = head_;
            Cell_t* next;

            // Out of cells to allocate
            if (!currentHead) {
                return allocateFromNewNode();
            }

            next = currentHead->next_;
            head_ = next;
            return &currentHead->val_;
        }

        void deallocate(void* p) noexcept {
            auto newHead = reinterpret_cast<Cell_t*>(p);
            Cell_t* currentHead = head_;
            newHead->next_ = currentHead;
            head_ = newHead;
        }

    private:
        static ChunkList<Size, AllocationsPerChunk> chunkList_;
        Cell_t* head_;

        void* allocateFromNewNode() {
            Cell_t* allocateCell;

            // Set head to the start of a new chain and get an
            //allocateable cell
            allocateCell = chunkList_.allocateChain(head_);

            return reinterpret_cast<void*>(&allocateCell->val_);
        }
    };

    template<std::size_t Size, std::size_t AllocationsPerChunk>
    ChunkList<Size, AllocationsPerChunk>
    Pool<Size, AllocationsPerChunk>::chunkList_;
}

#endif
