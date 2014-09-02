
#include <lfpAlloc/Allocator.hpp>
#include <gtest/gtest.h>
#include <list>
#include <thread>
#include <future>
#include <set>

TEST(AllocatorTest, Equality) {
    lfpAlloc::lfpAllocator<int, 8> allocator;
    lfpAlloc::lfpAllocator<int, 8> otherAllocator(allocator);

    EXPECT_EQ(allocator, otherAllocator);

    auto val = allocator.allocate(1);
    otherAllocator.deallocate(val, 1);
}

TEST(AllocatorTest, Allocate) {
    lfpAlloc::lfpAllocator<int> allocator;
    for (std::size_t s=0; s<5e6; ++s) {
        EXPECT_NE(allocator.allocate(1), nullptr);
    }
}

TEST(AllocatorTest, Distinct) {
    lfpAlloc::lfpAllocator<int> allocator;
    std::set<int*> prevVals;
    for (std::size_t s=0; s<5e4; ++s) {
        auto val = allocator.allocate(1);
        EXPECT_NE(val, nullptr);
        EXPECT_EQ(prevVals.count(val), 0);
        prevVals.insert(val);
    }
}

TEST(AllocatorTest, STLContainer) {
    std::list<int, lfpAlloc::lfpAllocator<int, 8>> l;
    std::vector<int, lfpAlloc::lfpAllocator<int, 8>> v;

    for (std::size_t s=0; s<5e5; ++s) {
        l.push_back(s);
        v.push_back(s);
    }
    EXPECT_TRUE(std::equal(l.begin(), l.end(), v.begin()));
}

TEST(AllocatorTest, Concurrent) {
    lfpAlloc::lfpAllocator<int, 8> allocator;
    auto future1 = std::async(std::launch::async, [&]{
        std::vector<int, lfpAlloc::lfpAllocator<int, 8>> v(allocator);
        for (std::size_t s=0; s<5e6; ++s) {
            v.push_back(s);
        }
        return v;
    });

    auto future2 = std::async(std::launch::async, [&]{
        std::vector<int, lfpAlloc::lfpAllocator<int, 8>> v(allocator);
        for (std::size_t s=0; s<5e6; ++s) {
            v.push_back(s);
        }
        return v;
    });

    std::vector<int> v;
    for (std::size_t s=0; s<5e6; ++s) {
        v.push_back(s);
    }

    auto result1 = future1.get();
    auto result2 = future2.get();
    EXPECT_TRUE(std::equal(v.begin(), v.end(), result1.begin()));
    EXPECT_TRUE(std::equal(result1.begin(), result1.end(), result2.begin()));
}

TEST(AllocatorTest, Alignment) {
    auto isAligned = [](void* p, int alignment) {
        return reinterpret_cast<uintptr_t>(p) % alignment == 0;
    };

    lfpAlloc::lfpAllocator<int, 8> intAlloc;
    for (std::size_t s=0; s < 5e4; ++s) {
        EXPECT_TRUE(isAligned(intAlloc.allocate(1), alignof(int)));
    }

    lfpAlloc::lfpAllocator<short, 8> shortAlloc(intAlloc);
    for (std::size_t s=0; s < 5e4; ++s) {
        EXPECT_TRUE(isAligned(shortAlloc.allocate(1), alignof(short)));
    }

    struct S{
        char c;
        float f;
    };

    lfpAlloc::lfpAllocator<S, 8> sAlloc(intAlloc);
    for (std::size_t s=0; s < 5e4; ++s) {
        EXPECT_TRUE(isAligned(sAlloc.allocate(1), alignof(S)));
    }
}

TEST(AllocatorTest, Move) {
    lfpAlloc::lfpAllocator<std::string> strAlloc;
    std::string* strPtr = strAlloc.allocate(1);
    EXPECT_NE(strPtr, nullptr);

    lfpAlloc::lfpAllocator<std::string> strAlloc2(std::move(strAlloc));

    lfpAlloc::lfpAllocator<int> intAlloc = std::move(strAlloc2);
    int* intPtr = intAlloc.allocate(1);
    EXPECT_NE(intPtr, nullptr);
}
