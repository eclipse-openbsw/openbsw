/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include <chrono>
#include <thread>
#include <unistd.h>

#include <sys/wait.h>

#include "Logger.h"
#include "ShmWrapper.h"

void run_main_core0();
void run_main_core1();

int main()
{
    middleware::shm::ShmWrapper shmWrapper;
    shmWrapper.init();

    // Construct the MemoryLayout (queues + allocator pools) in the shared memory region.
    middleware::shm::createMemoryLayout();

    int rc = 0;

#ifdef SIMULATION_USE_PROCESSES
    // Process-based execution: each core runs in its own OS process.
    // The shared memory region created above is accessible by all child processes.
    auto fork_child = [](void (*fn)()) -> pid_t
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            fn();
            exit(0);
        }
        return pid;
    };

    pid_t pid0 = fork_child(&run_main_core0);
    pid_t pid1 = fork_child(&run_main_core1);

    int status0 = 0;
    int status1 = 0;
    waitpid(pid0, &status0, 0);
    waitpid(pid1, &status1, 0);

    int const rc0 = (WIFEXITED(status0) != 0) ? WEXITSTATUS(status0) : 1;
    int const rc1 = (WIFEXITED(status1) != 0) ? WEXITSTATUS(status1) : 1;
    rc            = rc0 | rc1;

#else
    // Thread-based execution (default): cores share the same process address space.
    std::thread thread0{&run_main_core0};
    std::thread thread1{&run_main_core1};

    thread0.join();
    thread1.join();
#endif

    simulation::Logger::log("Simulation done.");

    return rc;
}
