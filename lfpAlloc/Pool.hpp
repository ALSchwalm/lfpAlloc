#ifndef LF_POOL_ALLOC_POOL
#define LF_POOL_ALLOC_POOL

#include <cstdint>
#include <atomic>
#include <memory>
#include <iostream>
#include <cassert>

namespace lfpAlloc {

    template<typename T, uint16_t CellsPerAllocation, std::size_t NumCells>
    class Pool {
    public:
        static_assert(NumCells % CellsPerAllocation == 0,
                      "NumCells must be a multiple of CellsPerAllocation.");

        Pool() : handle_(new Node_),
                 head_(&handle_.load()->memBlock_[0]){}

        ~Pool() {
            Node_* node = handle_.load();
            while(node) {
                auto temp = node;
                node = node->next_;
                delete temp;
            }
        }

        T* allocate(){
        start:
            Cell_* currentHead;
            Cell_* currentNext;

            Node_* currentHandle;
            Node_* newNode;

            // Ran out of cells to allocate
            if (!head_.load()) {

                // Make a new node
                newNode = new Node_;

                // Point the current Head's next to the head of the new node
                do {
                    currentHead = head_.load();
                    newNode->memBlock_[NumCells-CellsPerAllocation].next_ = currentHead;
                } while(!head_.compare_exchange_strong(currentHead, &newNode->memBlock_[0]));

                // Add the node to the chain
                do {
                    currentHandle = handle_.load();
                    newNode->next_ = currentHandle;
                } while(!handle_.compare_exchange_strong(currentHandle, newNode));
                assert(head_.load());
            }

            // Allocate by making head = head.next
            do {
                currentHead = head_.load();
                currentNext = currentHead->next_.load();
            } while (!head_.compare_exchange_strong(currentHead, currentNext));
            return reinterpret_cast<T*>(currentHead);
        }

        void deallocate(void* p) {
            auto newHead = reinterpret_cast<Cell_*>(p);
            Cell_* currentHead;
            do {
                currentHead = head_.load();
                newHead->next_ = currentHead;
            } while (!head_.compare_exchange_strong(currentHead, newHead));
        }

    private:
        union Cell_{
            std::atomic<Cell_*> next_;
            uint8_t val[sizeof(T)];
        };

        struct Node_ {
            Node_() {
                for (uint16_t s=0; s < NumCells/CellsPerAllocation; ++s) {
                    memBlock_[s*CellsPerAllocation].next_
                        .store(&memBlock_[(s+1)*CellsPerAllocation],
                               std::memory_order_relaxed);
                }
                auto& last = memBlock_[NumCells-CellsPerAllocation];
                last.next_.store(nullptr, std::memory_order_relaxed);
            }
            Cell_ memBlock_[NumCells];
            Node_* next_ = nullptr;
        };

        std::atomic<Node_*> handle_;
        std::atomic<Cell_*> head_;
    };
}

#endif
