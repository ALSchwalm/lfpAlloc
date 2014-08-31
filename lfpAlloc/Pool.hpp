#ifndef LF_POOL_ALLOC_POOL
#define LF_POOL_ALLOC_POOL

#include <cstdint>
#include <atomic>
#include <memory>
#include <type_traits>

namespace lfpAlloc {
    class Pool {
    public:
        Pool(std::size_t size, std::size_t chunkSize) :
            size_((size > sizeof(void*)) ? size : sizeof(void*)),
            chunkSize_(chunkSize),
            handle_(nullptr),
            head_(nullptr){}

        Pool(std::size_t size) :
            Pool(size, size*256) {}

        Pool() : Pool(0, 0){}

        Pool& operator=(const Pool& other) {
            size_ = other.size_;
            chunkSize_ = other.chunkSize_;
            handle_.store(other.handle_.load());
            head_.store(other.head_.load());
            return *this;
        }

        ~Pool() {
            Node_* node = handle_.load();
            while(node) {
                auto temp = node;
                node = node->next_;
                delete temp;
            }
        }

        void* allocate(){
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
                    newNode = new Node_(size_, chunkSize_);

                    auto first = asCell(newNode->memBlock_+size_);

                    // Point the current Head's next to the head of the new node
                    do {
                        currentHead = head_.load();
                        auto last = asCell(newNode->memBlock_+chunkSize_*size_);
                        last->next_ = currentHead;

                        // The first block is reserved for the current request
                    } while(!head_.compare_exchange_weak(currentHead, first));

                    // Add the node to the chain
                    do {
                        currentHandle = handle_.load();
                        newNode->next_ = currentHandle;
                    } while(!handle_.compare_exchange_weak(currentHandle, newNode));
                    return reinterpret_cast<void*>(&newNode->memBlock_[0]);
                }

                currentNext = withoutTag(currentHead)->next_.load();

                // Increment the tag by one
                tag = (reinterpret_cast<uintptr_t>(currentNext)+1) & 0x3;

                // Don't add tag to the nullptr
                if (currentNext) {
                    currentNext = addTag(currentNext, tag);
                }

            } while (!head_.compare_exchange_weak(currentHead, currentNext));
            return withoutTag(currentHead);
        }

        void deallocate(void* p) noexcept {
            auto newHead = reinterpret_cast<Cell_*>(p);
            Cell_* currentHead;

            // Deallocate by making newHead.next = head
            do {
                currentHead = head_.load(std::memory_order_relaxed);
                newHead->next_.store(currentHead, std::memory_order_relaxed);
            } while (!head_.compare_exchange_weak(currentHead, newHead));
        }

    private:
        struct Cell_{
            std::atomic<Cell_*> next_;
        };

        struct Node_ {
            Node_(std::size_t size_, std::size_t chunkSize_) :
                memBlock_(new uint8_t[size_*chunkSize_]) {
                for (std::size_t s=0; s < chunkSize_; ++s) {
                    asCell(memBlock_ + s*size_)->next_
                        .store(asCell(memBlock_+((s+1)*size_)),
                               std::memory_order_relaxed);
                }
                auto last = asCell(memBlock_+(size_*(chunkSize_-1)));
                last->next_.store(nullptr, std::memory_order_relaxed);
            }
            uint8_t* memBlock_;
            Node_* next_ = nullptr;
        };

        static inline Cell_* withoutTag(Cell_* const& cell) {
            return cell; //asCell((reinterpret_cast<uintptr_t>(cell) & ~0x3));
        }

        template<typename Tag_t>
        static inline Cell_* addTag(Cell_* const& cell, Tag_t tag) {
            return cell; //asCell((reinterpret_cast<uintptr_t>(cell) & ~0x3 | tag));
        }

        template<typename T>
        static inline Cell_* asCell(T p) {
            return reinterpret_cast<Cell_*>(p);
        }

        std::size_t size_;
        std::size_t chunkSize_;
        std::atomic<Node_*> handle_;
        std::atomic<Cell_*> head_;
    };
}

#endif
