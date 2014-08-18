#ifndef LF_POOL_ALLOC_CHUNK
#define LF_POOL_ALLOC_CHUNK

#include <atomic>
#include <limits>
#include <new>
#include <algorithm>

#ifndef ALLOW_LOCKING_ALLOC
static_assert(ATOMIC_POINTER_LOCK_FREE==2,
              "Allocator may block as atomic_pointer is potentially blocking.");
#endif

namespace lfpAlloc {

    template<typename T, uint16_t CellsPerAllocation, uint16_t NumCells>
    class PoolChunk{
    public:
        static constexpr auto MAX_CELLS = std::numeric_limits<uint16_t>::max();

        static_assert(NumCells % CellsPerAllocation == 0,
                      "NumCells must be a multiple of CellsPerAllocation.");

        PoolChunk() :
            head_(&memBlock_[0]){

            for (uint16_t s=0; s < NumCells/CellsPerAllocation; ++s) {
                memBlock_[s*CellsPerAllocation].next_
                    .store((s+1)*CellsPerAllocation, std::memory_order_relaxed);
            }
            auto& last = memBlock_[NumCells-CellsPerAllocation];
            last.next_.store(MAX_CELLS, std::memory_order_relaxed);
        }

        T* allocate(){
            Cell_* currentHead;
            Cell_* nextPointer;
            uint16_t currentNext;
            do {
                currentHead = head_.load();
                if (!currentHead) {
                    return nullptr;
                }
                currentNext = currentHead->next_.load();
                if (currentNext == MAX_CELLS) {
                    nextPointer = nullptr;
                } else {
                    nextPointer = &memBlock_[currentNext];
                }
            } while (!head_.compare_exchange_strong(currentHead, nextPointer));
            return reinterpret_cast<T*>(currentHead);
        }

        void deallocate(void* p){
            auto newHead = reinterpret_cast<Cell_*>(p);
            Cell_* currentHead;
            do {
                currentHead = head_.load();
                auto headIndex = std::distance(&memBlock_[0], newHead);
                newHead->next_ = headIndex;
            } while (!head_.compare_exchange_strong(currentHead, newHead));
        }

        bool contains(void* p) const {
            return p >= memBlock_ && p < memBlock_+NumCells;
        }

        constexpr uint16_t possibleAllocations() const {
            return NumCells/CellsPerAllocation;
        }

    private:
        union Cell_{
            std::atomic<uint16_t> next_;
            uint8_t val[sizeof(T)];
        };

        Cell_ memBlock_[NumCells];
        std::atomic<Cell_*> head_;
    };
}

#endif
