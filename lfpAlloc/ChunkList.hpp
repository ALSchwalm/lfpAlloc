#ifndef LF_POOL_ALLOC_CHUNK_LIST
#define LF_POOL_ALLOC_CHUNK_LIST

#include <cstdint>
#include <atomic>

#ifndef LFP_ALLOW_BLOCKING
static_assert(ATOMIC_POINTER_LOCK_FREE==2, "Atomic pointer is not lock-free.");
#endif

namespace lfpAlloc {

    template<std::size_t Size>
    struct Cell {
        uint8_t val_[Size];
        Cell* next_ = this+1;
    };

    template<std::size_t Size, std::size_t AllocationsPerChunk>
    struct Chunk {
        Chunk() noexcept {
            auto& last = memBlock_[AllocationsPerChunk-1];
            last.next_ = nullptr;
        }
        Cell<Size> memBlock_[AllocationsPerChunk];
    };

    template<typename T>
    struct Node {
        Node() : val_(), next_(nullptr){}
        Node(const T& t) : val_(t), next_(nullptr){}
        T val_;
        std::atomic<Node<T>*> next_;
    };

    template<std::size_t Size, std::size_t AllocationsPerChunk>
    class ChunkList {
        static constexpr auto CellSize = Size-sizeof(void*);
        using Chunk_t = Chunk<CellSize, AllocationsPerChunk>;
        using Cell_t = Cell<CellSize>;

        using ChunkNode = Node<Chunk_t>;
        using CellNode = Node<Cell_t*>;
    public:
        ChunkList() : handle_(nullptr),
                      head_(nullptr) {}

        Cell_t* allocateChain(Cell_t*& previousHead) {
            CellNode* recentHead = head_.load();
            CellNode* currentNext = nullptr;
            do {
                // If there are now available chains, allocate a new chunk
                if (!recentHead) {
                    ChunkNode* currentHandle;

                    // Make a new node
                    auto newChunk = new ChunkNode();

                    // Set the given head to the 1st cell. 0th is reserved for
                    // this allocation
                    previousHead = &newChunk->val_.memBlock_[1];

                    // Add the node to the chain
                    do {
                        currentHandle = handle_.load();
                        newChunk->next_ = currentHandle;
                    } while(!handle_.compare_exchange_weak(currentHandle, newChunk));
                    return &newChunk->val_.memBlock_[0];
                }

                currentNext = recentHead->next_;
            } while (!head_.compare_exchange_weak(recentHead, currentNext));

            previousHead = recentHead->val_;
            delete recentHead;

            if (previousHead->next_) {
                return previousHead->next_;
            } else {
                //TODO handle case where chain has only 1 item
                return nullptr;
            }
        }

        void deallocateChain(Cell_t* newCell) {
            if (!newCell) {
                return;
            }
            CellNode* currentHead = head_.load();

            // Construct a new node to be added to the linked list
            CellNode* newHead = new CellNode(newCell);

            // Add the chain to the linked list
            do {
                newHead->next_.store(currentHead, std::memory_order_release);
            } while (!head_.compare_exchange_weak(currentHead, newHead));
        }

    private:
        std::atomic<ChunkNode*> handle_;
        std::atomic<CellNode*> head_;
    };
}

#endif
