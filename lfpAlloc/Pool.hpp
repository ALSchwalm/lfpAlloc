#ifndef LF_POOL_ALLOC_POOL
#define LF_POOL_ALLOC_POOL

#include <lfpAlloc/Utils.hpp>
#include <lfpAlloc/ChunkList.hpp>

namespace lfpAlloc {
    template<std::size_t Size, std::size_t AllocationsPerChunk>
    class Pool {
    public:
        static constexpr std::size_t size = Size-sizeof(void*);
        using Chunk_t = Chunk<Size, AllocationsPerChunk>;
        using Cell_t = Cell<Size>;

        Pool() : handle_(nullptr),
                 head_(nullptr){}

        ~Pool() {
            Chunk_t* node = handle_;
            // TODO: deallocate to inter-thread pool of nodes
            while(node) {
                auto temp = node;
                node = node->next_;
                delete temp;
            }
        }

        void* allocate(){
            // Head loaded from head_
            Cell_t* currentHead = head_;
            Cell_t* next;

            // Out of cells to allocate
            if (!currentHead) {
                return allocateFromNewNode(currentHead);
            }

            next = currentHead->next_;
            head_ = next;
            return currentHead;
        }

        void deallocate(void* p) noexcept {
            auto newHead = reinterpret_cast<Cell_t*>(p);
            Cell_t* currentHead = head_;
            newHead->next_ = currentHead;
            head_ = newHead;
        }

    private:
        static ChunkList<Size> chunkList_;
        Chunk_t* handle_;
        Cell_t* head_;

        void* allocateFromNewNode(Cell_t*& currentHead) {
            Chunk_t* currentHandle;
            Chunk_t* newNode;

            // Connect new node to current node
            // TODO: Allocate from inter-thread pool of nodes
            newNode = new Chunk_t();
            newNode->memBlock_[AllocationsPerChunk-1].next_ = currentHead;

            // Set head to 1st block (0 is reserved for this allocation)
            head_ = &newNode->memBlock_[1];

            // Add new node to chain of nodes
            currentHandle = handle_;
            newNode->next_ = currentHandle;
            handle_ = newNode;
            return reinterpret_cast<void*>(&newNode->memBlock_[0]);
        }
    };
}

#endif
