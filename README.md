lfpAlloc
========

lfpAlloc is a lock-free pool based allocator written using C++11 concurrency features.

Installation
============

The library is headers-only, so it may be used by simply executing `git clone https://github.com/ALSchwalm/lfpAlloc.git` and adding the `lfpAlloc` folder to your projects include path.

Tests may be compiled as executed by running `make` from the project root.

Requirements
============

In addition to a standards compliant C++11 compiler, lfpAlloc also requires the following:

- The implementation of `std::atomic<void*>` must be non-blocking. That is `ATOMIC_POINTER_LOCK_FREE` must be defined as 2. The user may override this requirement by defining `LFP_ALLOW_BLOCKING`.
