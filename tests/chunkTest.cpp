
#include <lfpAlloc/PoolChunk.hpp>
#include <gtest/gtest.h>

TEST(ChunkTest, PossibleAllocations) {
    lfpAlloc::PoolChunk<int, 1, 100> intChunk;
    EXPECT_EQ(intChunk.possibleAllocations(), 100);

    lfpAlloc::PoolChunk<float, 2, 50> floatChunk;
    EXPECT_EQ(floatChunk.possibleAllocations(), 25);
}

TEST(ChunkTest, Allocate) {
    lfpAlloc::PoolChunk<int, 1, 100> intChunk;
    int* intPtr1 = static_cast<int*>(intChunk.allocate());
    int* intPtr2 = static_cast<int*>(intChunk.allocate());

    EXPECT_NE(intPtr1, intPtr2);

    lfpAlloc::PoolChunk<std::string, 1, 10> stringChunk;
    std::string* strPtr1 = static_cast<std::string*>(stringChunk.allocate());
    strPtr1 = new (strPtr1) std::string("My Test String");
    EXPECT_EQ(*strPtr1, "My Test String");
}

TEST(ChunkTest, CellsPerAllocation) {
    lfpAlloc::PoolChunk<int, 2, 100> intChunk;
    for (std::size_t s=0; s < intChunk.possibleAllocations(); ++s) {
        EXPECT_NE(intChunk.allocate(), nullptr);
    }
    EXPECT_EQ(intChunk.allocate(), nullptr);

    lfpAlloc::PoolChunk<int, 4, 100> intChunk2;
    for (std::size_t s=0; s < intChunk2.possibleAllocations(); ++s) {
        EXPECT_NE(intChunk2.allocate(), nullptr);
    }
    EXPECT_EQ(intChunk2.allocate(), nullptr);
}

TEST(ChunkTest, AvailableAllocations) {
    lfpAlloc::PoolChunk<int, 1, 100> chunk;
    for (std::size_t s=0; s < 100; ++s) {
        EXPECT_NE(chunk.allocate(), nullptr);
    }
    EXPECT_EQ(chunk.allocate(), nullptr);

    lfpAlloc::PoolChunk<int, 2, 10> smallChunk;
    for (std::size_t s=0; s < 5; ++s) {
        EXPECT_NE(smallChunk.allocate(), nullptr);
    }
    EXPECT_EQ(smallChunk.allocate(), nullptr);
}

TEST(ChunkTest, Contains) {
    lfpAlloc::PoolChunk<int, 1, 10> chunk1;
    int* ptr1 = static_cast<int*>(chunk1.allocate());

    lfpAlloc::PoolChunk<int, 1, 10> chunk2;
    int* ptr2 = static_cast<int*>(chunk2.allocate());

    EXPECT_TRUE(chunk1.contains(ptr1));
    EXPECT_TRUE(chunk2.contains(ptr2));
    EXPECT_FALSE(chunk1.contains(ptr2));
    EXPECT_FALSE(chunk2.contains(ptr1));
    EXPECT_FALSE(chunk1.contains(nullptr));
}
