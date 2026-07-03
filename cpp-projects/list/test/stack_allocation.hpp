#pragma once

#include "pool_allocator.hpp"

#include <cstddef>
#include <iostream>

#include <sys/resource.h>

constexpr size_t kStorageSize = 120 * 1024 * 1024;  // 120 MB
PoolStorage<kStorageSize> static_storage;

constexpr rlim_t kStackSize = 250 * 1024 * 1024;  // 250MB, min stack size = 16 MB

#if !defined(__APPLE__)
struct StackProvider {
    StackProvider() {
        struct rlimit rl;
        int result;

        result = getrlimit(RLIMIT_STACK, &rl);
        if (result != 0) {
            std::cerr << "Failed to get current stack size\n";
            std::abort();
        }

        std::cerr << "Limit: " << rl.rlim_cur << '\n';

        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0) {
                std::cerr << "Failed to set bigger stack size\n";
                std::abort();
            }
            std::cerr << "Stack size is successfully set to " << kStackSize << '\n';
        } else {
            std::cerr << "Stack size is left at " << rl.rlim_cur << '\n';
        }
    }
} stack_provider;
#endif
