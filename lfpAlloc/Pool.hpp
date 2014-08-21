#ifndef LF_POOL_ALLOC_POOL
#define LF_POOL_ALLOC_POOL

#include <cstdint>
#include <atomic>
#include <memory>

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
            Cell_* currentHead;
            Cell_* nextPointer;
            do {
                currentHead = head_.load();
                nextPointer = currentHead->next_.load();
                if (!nextPointer) {
                    auto newNode = new Node_;
                    Node_* currentHandle;
                    do {
                        currentHandle = handle_.load();
                        newNode->next_ = currentHandle;
                    } while (!handle_.compare_exchange_strong(currentHandle, newNode));
                    nextPointer = &newNode->memBlock_[0];
                }
            } while (!head_.compare_exchange_strong(currentHead, nextPointer));
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
