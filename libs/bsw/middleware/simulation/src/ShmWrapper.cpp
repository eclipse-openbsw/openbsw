/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "ShmWrapper.h"

#include <cerrno>
#include <cstring>
#include <iostream>

#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include <unistd.h>   /* For ftruncate */

#include "middleware/shm/Config.h"

namespace middleware
{
namespace shm
{

uint8_t* gShmAddr = nullptr;
size_t gShmSize   = 0;

static MemoryLayout* gMemoryLayout = nullptr;

char const* shm_path = "/mware_shm";

size_t const SHM_SIZE = 32UL * 1024UL;

uint8_t* create_shm_region(char const* path, int& fd, size_t size, uint8_t* pointer, int flags)
{
    fd = shm_open(path, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (fd < 0)
    {
        std::cout << "shm_open failed for " << path << " with errno: " << strerror(errno) << "\n";
        return nullptr;
    }

    if (ftruncate(fd, static_cast<off_t>(size)) == -1)
    {
        std::cout << "ftruncate failed for " << path << " with errno: " << strerror(errno) << "\n";
        close(fd);
        fd = -1;
        return nullptr;
    }

    pointer = static_cast<uint8_t*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (pointer == MAP_FAILED)
    {
        std::cout << "mmap failed for " << path << " with errno: " << strerror(errno) << "\n";
        close(fd);
        fd = -1;
        return nullptr;
    }

    return pointer;
}

void destroy_shm_region(char const* path, int& fd, size_t size, uint8_t* pointer)
{
    if (munmap(pointer, size) < 0)
    {
        std::cout << "munmap failed for " << path << " with errno: " << strerror(errno) << "\n";
    }
    shm_unlink(path);
    close(fd);
}

ShmWrapper::ShmWrapper() {}

ShmWrapper::~ShmWrapper() { deInit(); }

void ShmWrapper::init()
{
    int const flags = O_CREAT | O_RDWR;
    gShmAddr        = create_shm_region(shm_path, shm_fd_, SHM_SIZE, gShmAddr, flags);

    if (gShmAddr == nullptr)
    {
        std::cout << "Failed to create shared memory regions.\n";
        return;
    }
    gShmSize = SHM_SIZE;

    static_assert(
        SHM_SIZE >= sizeof(MemoryLayout),
        "SHM_SIZE is too small to hold MemoryLayout — increase it.");

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    gMemoryLayout = reinterpret_cast<MemoryLayout*>(gShmAddr);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    std::cout << "Successfully created shared memory regions.\n";
}

void ShmWrapper::deInit() { destroy_shm_region(shm_path, shm_fd_, SHM_SIZE, gShmAddr); }

// Declared in config.h
MemoryLayout& getMemoryLayout()
{
    if (gMemoryLayout == nullptr)
    {
        std::cerr << "getMemoryLayout() called before ShmWrapper::init()\n";
        std::abort();
    }
    return *gMemoryLayout;
}

// Declared in shm_wrapper.h
void createMemoryLayout()
{
    if (gMemoryLayout == nullptr)
    {
        std::cerr << "createMemoryLayout() called before ShmWrapper::init()\n";
        std::abort();
    }
    new (gMemoryLayout) MemoryLayout();
}

} // namespace shm
} // namespace middleware
