#ifndef LF_POOL_ALLOC_POOL
#define LF_POOL_ALLOC_POOL

#include <cstdint>
#include <atomic>
#include <memory>
#include <type_traits>
#include <lfpAlloc/Utils.hpp>

#ifndef LFP_ALLOW_BLOCKING
static_assert(ATOMIC_POINTER_LOCK_FREE==2, "Atomic pointer is not lock-free.");
#endif

namespace lfpAlloc {
    template<std::size_t Size, std::size_t AllocationsPerChunk>
    class Pool {
    public:
        static_assert(Size >= sizeof(void*), "Invalid pool size.");
        static constexpr std::size_t size = Size;

        Pool() : handle_(nullptr),
                 head_(nullptr){}

        ~Pool() {
            Node_* node = handle_;
            // TODO: deallocate to inter-thread pool of nodes
            while(node) {
                auto temp = node;
                node = node->next_;
                delete temp;
            }
        }

        void* allocate(){
            // Head atomic loaded from head_
            Cell_* currentHead = head_;
            Cell_* next;

            // Out of cells to allocate
            if (!currentHead) {
                return allocateFromNewNode(currentHead);
            }

            next = currentHead->next_;
            head_ = next;
            return currentHead;
        }

        void deallocate(void* p) noexcept {
            auto newHead = reinterpret_cast<Cell_*>(p);
            Cell_* currentHead = head_;
            newHead->next_ = currentHead;
            head_ = newHead;
        }

    private:
        union Cell_{
            Cell_* next_ = this+1;
            uint8_t val[size];
        };

        struct Node_ {
            Node_() noexcept {
                auto& last = memBlock_[AllocationsPerChunk-1];
                last.next_ = nullptr;
            }
            Cell_ memBlock_[AllocationsPerChunk];
            Node_* next_ = nullptr;
        };

        inline void* allocateFromNewNode(Cell_*& currentHead) {
            Node_* currentHandle;
            Node_* newNode;

            // Connect new node to current node
            // TODO: Allocate from inter-thread pool of nodes
            newNode = new Node_();
            newNode->memBlock_[AllocationsPerChunk-1].next_ = currentHead;

            // Set head to 1st block (0 is reserved for this allocation)
            head_ = &newNode->memBlock_[1];

            // Add new node to chain of nodes
            currentHandle = handle_;
            newNode->next_ = currentHandle;
            handle_ = newNode;
            return reinterpret_cast<void*>(&newNode->memBlock_[0]);
        }

        Node_* handle_;
        Cell_* head_;
    };
}

#endif
