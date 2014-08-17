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

    template<typename T, std::size_t CellsPerAllocation, std::size_t NumCells>
    class PoolChunk{
    public:

        static_assert(NumCells % CellsPerAllocation == 0,
                      "NumCells must be a multiple of CellsPerAllocation.");

        PoolChunk() :
            allocatedCells_(0),
            head_(&memBlock_[0]){

            for (uint16_t s=0; s < NumCells/CellsPerAllocation; ++s) {
                memBlock_[s*CellsPerAllocation].next_ =
                    (s+1)*CellsPerAllocation;
            }
            auto& last = memBlock_[NumCells-CellsPerAllocation];
            last.next_.store(std::numeric_limits<uint16_t>::max());
        }

        T* allocate(){
            Cell_* currentHead;
            uint16_t currentNext;
            do {
                if (isFull()) {
                    throw std::bad_alloc();
                }

                currentHead = head_.load();
                currentNext = currentHead->next_.load();
            } while (!head_.compare_exchange_strong(currentHead, &memBlock_[currentNext]));
            allocatedCells_+=CellsPerAllocation;
            return reinterpret_cast<T*>(currentHead);
        }

        void deallocate(void* p){
            auto newHead = reinterpret_cast<Cell_*>(p);
            Cell_* currentHead;
            do {
                if (isEmpty()) {
                    throw std::bad_alloc();
                }
                currentHead = head_.load();
                auto headIndex = std::distance(&memBlock_[0], newHead);
                newHead->next_.store(headIndex);
            } while (!head_.compare_exchange_strong(currentHead, newHead));
            allocatedCells_-=CellsPerAllocation;
        }

        bool contains(void* p) const {
            return p >= memBlock_ && p < memBlock_+NumCells;
        }

        bool isFull() const {
            return allocatedCells_.load() == NumCells;
        }

        bool isEmpty() const {
            return !allocatedCells_.load();
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
        std::atomic<uint16_t> allocatedCells_;
        std::atomic<Cell_*> head_;
    };
}

#endif
