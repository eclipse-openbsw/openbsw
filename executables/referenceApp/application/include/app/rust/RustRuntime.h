/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <async/Types.h>

namespace app
{
/**
 * Hosts a Rust (Embassy) async runtime inside a single C++ Async task.
 */
class RustRuntime : private ::async::RunnableType
{
public:
    explicit RustRuntime(::async::ContextType context);

    /**
     * Create the Rust runtime and run it once. Call once, from the hosting task.
     */
    void init();

    /**
     * Reschedule a poll on the hosting task. Called from the Rust wake path.
     */
    void schedule();

private:
    void execute() override;

    ::async::ContextType _context;
};

} // namespace app
