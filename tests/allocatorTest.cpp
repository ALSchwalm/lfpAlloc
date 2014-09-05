
#include <lfpAlloc/Allocator.hpp>
#include <gtest/gtest.h>
#include <list>
#include <thread>
#include <future>
#include <set>
#include <type_traits>

using lfpAlloc::lfpAllocator;

TEST(AllocatorTest, TypeTraits) {
    static_assert(std::is_nothrow_copy_constructible<lfpAllocator<int>>::value,
                  "No copy constructor");

    static_assert(std::is_nothrow_move_constructible<lfpAllocator<int>>::value,
                  "No move constructor");

    static_assert(std::is_nothrow_move_assignable<lfpAllocator<int>>::value,
                  "No move assignment operator");

    static_assert(std::is_nothrow_copy_assignable<lfpAllocator<int>>::value,
                  "No copy assignment operator");
}

TEST(AllocatorTest, Equality) {
    lfpAllocator<int, 8> allocator;
    lfpAllocator<int, 8> copyAllocator(allocator);
    lfpAllocator<float, 8> differentAllocator;

    EXPECT_EQ(allocator, copyAllocator);
    EXPECT_NE(allocator, differentAllocator);

    auto val = allocator.allocate(1);
    copyAllocator.deallocate(val, 1);
}

TEST(AllocatorTest, Allocate) {
    lfpAllocator<int> allocator;
    for (std::size_t s=0; s<5e6; ++s) {
        EXPECT_NE(allocator.allocate(1), nullptr);
    }
}

TEST(AllocatorTest, Distinct) {
    lfpAllocator<int> allocator;
    std::set<int*> prevVals;
    for (std::size_t s=0; s<5e4; ++s) {
        auto val = allocator.allocate(1);
        EXPECT_NE(val, nullptr);
        EXPECT_EQ(prevVals.count(val), 0);
        prevVals.insert(val);
    }
}

TEST(AllocatorTest, STLContainer) {
    std::list<int, lfpAllocator<int, 8>> l;
    std::vector<int, lfpAllocator<int, 8>> v;

    for (std::size_t s=0; s<5e5; ++s) {
        l.push_back(s);
        v.push_back(s);
    }
    EXPECT_TRUE(std::equal(l.begin(), l.end(), v.begin()));
}

TEST(AllocatorTest, Concurrent) {
    lfpAllocator<int, 8> allocator;
    auto future1 = std::async(std::launch::async, [&]{
        std::vector<int, lfpAllocator<int, 8>> v(allocator);
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

    lfpAllocator<int, 8> intAlloc;
    for (std::size_t s=0; s < 5e4; ++s) {
        EXPECT_TRUE(isAligned(intAlloc.allocate(1), alignof(int)));
    }

    lfpAllocator<short, 8> shortAlloc(intAlloc);
    for (std::size_t s=0; s < 5e4; ++s) {
        EXPECT_TRUE(isAligned(shortAlloc.allocate(1), alignof(short)));
    }

    struct S{
        char c;
        float f;
    };

    lfpAllocator<S, 8> sAlloc(intAlloc);
    for (std::size_t s=0; s < 5e4; ++s) {
        EXPECT_TRUE(isAligned(sAlloc.allocate(1), alignof(S)));
    }
}

TEST(AllocatorTest, Move) {
    lfpAllocator<std::string> strAlloc;
    std::string* strPtr = strAlloc.allocate(1);
    EXPECT_NE(strPtr, nullptr);

    lfpAllocator<std::string> strAlloc2(std::move(strAlloc));

    lfpAllocator<int> intAlloc = std::move(strAlloc2);
    int* intPtr = intAlloc.allocate(1);
    EXPECT_NE(intPtr, nullptr);
}
