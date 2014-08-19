#ifndef LF_POOL_ALLOC_POOL
#define LF_POOL_ALLOC_POOL

#include <cstdint>
#include <atomic>
#include <lfpAlloc/PoolChunk.hpp>
#include <iostream> //TODO remove this

namespace lfpAlloc {

    template<typename T, uint16_t CellsPerAllocation, uint16_t NumCells>
    class Pool {
    public:
        Pool() : head_(new Node_){
            head_.load()->next_.store(head_);
        }

        T* allocate(){
            Node_* recentHead = head_.load();
            T* val = nullptr;

            // Try to allocate from the active (head) chunk
            if (!(val = recentHead->chunk_.allocate())) {
                Node_* next = recentHead->next_.load();

                // Try to find a chunk with free cells
                while(next != recentHead) {
                    if (val = next->chunk_.allocate()) {
                        return val;
                    }
                    next = next->next_.load();
                }

                // Add a new chunk to the ring
                Node_* newNode = new Node_;

                // Allocate before inserting the chunk into the ring
                val = newNode->chunk_.allocate();

                Node_* currentNext;
                do {
                    currentNext = head_.load()->next_.load();
                    newNode->next_.store(recentHead->next_);
                } while (!head_.load()->next_.compare_exchange_strong(currentNext, newNode));

                // Point the head at the new node in the ring
                head_.store(newNode);
                return val;
            } else {
                return val;
            }
        }

        void deallocate(void* p){
            Node_* recentHead = head_.load();
            Node_* next = recentHead->next_.load();

            while(next != recentHead) {
                if (next->chunk_.contains(p)) {
                    next->chunk_.deallocate(p);
                    return;
                }
                next = next->next_.load();
            }
        }

    private:
        struct Node_ {
            PoolChunk<T, CellsPerAllocation, NumCells> chunk_;
            std::atomic<Node_*> next_;
        };

        std::atomic<Node_*> head_;
    };
}

#endif
