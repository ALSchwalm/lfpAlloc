#ifndef LF_POOL_ALLOC_CHUNK
#define LF_POOL_ALLOC_CHUNK

#include <cstddef>
#include <atomic>
#include <limits>
#include <new>
#include <algorithm>

#ifndef ALLOW_LOCKING_ALLOC
static_assert(ATOMIC_POINTER_LOCK_FREE==2,
              "Allocator may block as atomic_pointer is potentially blocking.");
#endif

namespace lfPoolAlloc {

    template<typename T>
    class LFPoolChunck{
    public:
        LFPoolChunck(uint16_t cellsPerAllocation,
                     uint16_t numCells) :
            cellsPerAllocation_(cellsPerAllocation),
            numCells_(numCells),
            memBlock_(new Cell_[numCells_]),
            head_(memBlock_),
            allocatedCells_(0){

            for (uint16_t s=0; s < numCells_-1; ++s) {
                memBlock_[s].next_ = s+1;
            }
            memBlock_[numCells_-1].next_.store(std::numeric_limits<uint16_t>::max());
        }

        ~LFPoolChunck() {
            delete[] memBlock_;
        }

        void* allocate(){
            Cell_* currentHead;
            uint16_t currentNext;
            do {
                if (isFull()) {
                    throw std::bad_alloc();
                }

                currentHead = head_.load();
                currentNext = currentHead->next_.load();
            } while (!head_.compare_exchange_strong(currentHead, &memBlock_[currentNext]));
            ++allocatedCells_;
            return currentHead;
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
            --allocatedCells_;
        }

        bool contains(void* p) const {
            return p >= memBlock_ && p < memBlock_+numCells_;
        }

        bool isFull() const {
            return allocatedCells_.load() == numCells_;
        }

        bool isEmpty() const {
            return !allocatedCells_.load();
        }

    private:
        union Cell_{
            std::atomic<uint16_t> next_;
            uint8_t val[sizeof(T)];
        };

        const uint16_t cellsPerAllocation_;
        const uint16_t numCells_;
        Cell_* memBlock_;
        std::atomic<Cell_*> head_;
        std::atomic<uint16_t> allocatedCells_;
    };
}

#endif
