///////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Accenture
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0
//
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////////////////////////
use openbsw_async::runtime;

#[unsafe(no_mangle)]
pub extern "C" fn rust_runtime_init(task_id: u8) {
    // `runtime::init` only spawns (registers) the tasks — it runs no task code —
    // so it is safe to call here, during startup.
    runtime::init(task_id, |spawner| {
        crate::async_tasks::spawn(spawner, task_id);
    });
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_runtime_poll(task_id: u8) {
    // SAFETY: the C++ host only ever calls this from the runnable of the task
    // that hosts `task_id` (RustRuntime::execute), which the async framework
    // never re-enters — so the runtime is polled from exactly one task,
    // non-reentrantly (see rust_runtime.h).
    unsafe { runtime::poll(task_id) };
}

struct CppCriticalSection;

critical_section::set_impl!(CppCriticalSection);

unsafe impl critical_section::Impl for CppCriticalSection {
    unsafe fn acquire() -> u32 {
        unsafe { rust_critical_section_acquire() }
    }

    unsafe fn release(restore_state: u32) {
        unsafe { rust_critical_section_release(restore_state) }
    }
}

unsafe extern "C" {
    fn rust_critical_section_acquire() -> u32;
    fn rust_critical_section_release(restore_state: u32);
}
