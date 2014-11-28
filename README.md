lfpAlloc
========

lfpAlloc is a lock-free pool based allocator written using C++11 concurrency features.

Installation
============

The library is headers-only, so it may be used by simply executing `git clone https://github.com/ALSchwalm/lfpAlloc.git` and adding the `lfpAlloc` folder to your project's include path.

Tests may be compiled and executed by running `make` from the project root.

Usage
-----

lfpAllocator exposes a standard C++ allocator interface. That is, an allocator for a type `T` may be constructed with `lfpAllocator<T> alloc`. Correctly aligned space for an instance of `T` may then be allocated with `alloc.allocate(1)`. Similarly, a pointer previously allocated from an equivalent lfpAllocator may be deallocated with `alloc.deallocate(p, 1)`.

STL containers all take an allocator type as a template parameter. For example, an STL list which allocates its memory from lfpAllocator may be constructed with `std::list<T, lfpAllocator<T>> l`.

Requirements
============

In addition to a standards compliant C++11 compiler, lfpAlloc also requires the following:

- The implementation of `std::atomic<void*>` must be non-blocking. That is `ATOMIC_POINTER_LOCK_FREE` must be defined as 2. The user may override this requirement by defining `LFP_ALLOW_BLOCKING`.
