
#include <lfPoolAlloc/PoolChunk.hpp>
#include <gtest/gtest.h>

TEST(ChunkTest, Allocate) {
    lfPoolAlloc::LFPoolChunck<int> intChunk(1, 100);
    int* intPtr1 = static_cast<int*>(intChunk.allocate());
    int* intPtr2 = static_cast<int*>(intChunk.allocate());

    EXPECT_NE(intPtr1, intPtr2);

    lfPoolAlloc::LFPoolChunck<std::string> stringChunk(1, 10);
    std::string* strPtr1 = static_cast<std::string*>(stringChunk.allocate());
    strPtr1 = new (strPtr1) std::string("My Test String");
    EXPECT_EQ(*strPtr1, "My Test String");
}

TEST(ChunkTest, AvailableAllocations) {
    lfPoolAlloc::LFPoolChunck<int> chunk(1, 100);
    for (std::size_t s=0; s < 100; ++s) {
        EXPECT_NO_THROW(chunk.allocate());
    }
    EXPECT_THROW(chunk.allocate(), std::bad_alloc);

    lfPoolAlloc::LFPoolChunck<int> smallChunk(1, 10);
    for (std::size_t s=0; s < 10; ++s) {
        EXPECT_NO_THROW(smallChunk.allocate());
    }
    EXPECT_THROW(smallChunk.allocate(), std::bad_alloc);
}

TEST(ChunkTest, Contains) {
    lfPoolAlloc::LFPoolChunck<int> chunk1(1, 10);
    int* ptr1 = static_cast<int*>(chunk1.allocate());

    lfPoolAlloc::LFPoolChunck<int> chunk2(1, 10);
    int* ptr2 = static_cast<int*>(chunk2.allocate());

    EXPECT_TRUE(chunk1.contains(ptr1));
    EXPECT_TRUE(chunk2.contains(ptr2));
    EXPECT_FALSE(chunk1.contains(ptr2));
    EXPECT_FALSE(chunk2.contains(ptr1));
    EXPECT_FALSE(chunk1.contains(nullptr));
}
