#ifndef LF_POOL_ALLOC_POOL
#define LF_POOL_ALLOC_POOL

#include <cstdint>
#include <atomic>
#include <memory>
#include <type_traits>
#include <iostream>

namespace lfpAlloc {

    template<typename T, std::size_t CellsPerAllocation, std::size_t NumCells>
    class Pool {
    public:
        static_assert(NumCells % CellsPerAllocation == 0,
                      "NumCells must be a multiple of CellsPerAllocation.");

        Pool() : handle_(nullptr),
                 head_(nullptr){}

        ~Pool() {
            Node_* node = handle_.load();
            while(node) {
                auto temp = node;
                node = node->next_;
                delete temp;
            }
        }

        T* allocate(){
            // Head atomic loaded from head_
            Cell_* currentHead;

            uintptr_t tag;
            Cell_* currentNext;

            // Allocate by making head = head.next
            do {
                currentHead = head_.load();

                // Out of cells to allocate
                if (!currentHead) {
                    Node_* currentHandle;
                    Node_* newNode;

                    // Make a new node
                    newNode = new Node_;

                    // Point the current Head's next to the head of the new node
                    do {
                        currentHead = head_.load();
                        newNode->memBlock_[NumCells-CellsPerAllocation].next_ = currentHead;

                        // The first block is reserved for the current request
                    } while(!head_.compare_exchange_weak(currentHead, &newNode->memBlock_[1]));

                    // Add the node to the chain
                    do {
                        currentHandle = handle_.load();
                        newNode->next_ = currentHandle;
                    } while(!handle_.compare_exchange_weak(currentHandle, newNode));
                    return reinterpret_cast<T*>(&newNode->memBlock_[0]);
                }

                currentNext = withoutTag(currentHead)->next_.load();
                tag = (reinterpret_cast<uintptr_t>(currentNext)+1) & 0x3;

                // Don't add tag to the nullptr
                if (currentNext) {
                    currentNext = addTag(currentNext, tag);
                }

            } while (!head_.compare_exchange_weak(currentHead, currentNext));
            return reinterpret_cast<T*>(currentHead);
        }

        void deallocate(void* p) noexcept {
            auto newHead = reinterpret_cast<Cell_*>(p);
            Cell_* currentHead;

            // Deallocate by making newHead.next = head
            do {
                currentHead = head_.load();
                newHead->next_.store(currentHead);
            } while (!head_.compare_exchange_weak(currentHead, newHead));
        }

    private:
        union Cell_{
            std::atomic<Cell_*> next_;
            uint8_t val[sizeof(T)];
        };

        struct Node_ {
            Node_() noexcept {
                for (std::size_t s=0; s < NumCells/CellsPerAllocation; ++s) {
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

        inline Cell_* withoutTag(Cell_* const& cell) const {
            return reinterpret_cast<Cell_*>((reinterpret_cast<uintptr_t>(cell) & ~0x3));
        }

        template<typename Tag_t>
        inline Cell_* addTag(Cell_* const& cell, Tag_t tag) const {
            return reinterpret_cast<Cell_*>((reinterpret_cast<uintptr_t>(cell) & ~0x3) | tag);
        }

        std::atomic<Node_*> handle_;
        std::atomic<Cell_*> head_;
    };
}

#endif
