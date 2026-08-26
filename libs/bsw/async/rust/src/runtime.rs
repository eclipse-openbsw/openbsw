///////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Accenture
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0
//
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////////////////////////

//! Multi-executor host for driving Embassy tasks cooperatively from C++ Async
//! tasks.
//!
//! Embassy ties one [`crate::HostedExecutor`] to exactly one driving task, so each
//! RTOS task / priority level that runs Rust async tasks gets its own executor.
//!
//! * [`init`] — create the executor for a hosting task and spawn its initial
//!   task(s).
//! * [`poll`] — poll the executor hosting a given task id once.

use embassy_executor::Spawner;

use crate::HostedExecutor;

/// Maximum number of concurrently hosted executors.
pub const MAX_RUNTIMES: usize = {
    if cfg!(feature = "runtimes-16") {
        16
    } else if cfg!(feature = "runtimes-8") {
        8
    } else if cfg!(feature = "runtimes-4") {
        4
    } else if cfg!(feature = "runtimes-2") {
        2
    } else if cfg!(feature = "runtimes-1") {
        1
    } else {
        4
    }
};

/// One pool entry: the executor and the task id it is hosted on.
struct Slot {
    executor: Option<HostedExecutor>,
    task_id: u8,
}

/// `'static` storage for the per-task executors.
///
/// Embassy keeps references to an executor for the whole lifetime of the
/// program, so each one must live forever, this array provides that storage. A
/// slot is written exactly once, during its hosting task's first activation, and
/// is never moved or cleared afterwards.
static mut SLOTS: [Slot; MAX_RUNTIMES] = [const {
    Slot {
        executor: None,
        task_id: 0,
    }
}; MAX_RUNTIMES];

static mut SLOT_COUNT: usize = 0;

/// Create the executor for `task_id` and spawn its initial task(s) via
/// `on_init`.
///
/// Call once per hosting task / priority level before any [`poll`] for that
/// task. The `on_init` closure receives the executor's [`Spawner`] and should
/// spawn the task(s) that run on this executor. This only registers the
/// tasks, it runs no task code, so it is safe to call from any context.
///
/// # Panics
///
/// Panics if an executor has already been initialised for `task_id`, or if more
/// than [`MAX_RUNTIMES`] executors are requested.
pub fn init<F: FnOnce(Spawner)>(task_id: u8, on_init: F) {
    let executor_ptr = critical_section::with(|_| {
        // SAFETY: we are inside a critical section, so forming temporary
        // references to the `'static` pool here is sound.
        unsafe {
            let slots = &mut *core::ptr::addr_of_mut!(SLOTS);
            let count_ptr = core::ptr::addr_of_mut!(SLOT_COUNT);
            let count = *count_ptr;

            for slot in slots.iter().take(count) {
                assert!(
                    slot.task_id != task_id,
                    "Rust runtime already initialized for this task id"
                );
            }
            assert!(count < MAX_RUNTIMES, "too many Rust runtimes");

            let slot = &mut slots[count];
            slot.task_id = task_id;
            slot.executor = Some(HostedExecutor::new(task_id));
            *count_ptr = count + 1;
            slot.executor.as_mut().unwrap() as *mut HostedExecutor
        }
    });

    // SAFETY: `executor_ptr` points into the `'static` pool and was just written
    // above; forming a shared `'static` reference to seed it is sound.
    let executor: &'static HostedExecutor = unsafe { &*executor_ptr };
    executor.init(on_init);
}

/// Poll the executor hosting `task_id` once.
///
/// # Safety
///
/// Must only be called from the single task that hosts `task_id`, and never
/// re-entrantly (Embassy's executor is not re-entrant). This upholds the
/// contract of [`HostedExecutor::poll`].
pub unsafe fn poll(task_id: u8) {
    // SAFETY: slots are written once and never moved, and by this function's
    // contract the executor is only polled from its own hosting task, so forming
    // these shared `'static` borrows cannot alias a live `&mut`.
    unsafe {
        let slots = &*core::ptr::addr_of!(SLOTS);
        let count = *core::ptr::addr_of!(SLOT_COUNT);
        for slot in slots.iter().take(count) {
            if slot.task_id == task_id {
                if let Some(executor) = slot.executor.as_ref() {
                    let executor: &'static HostedExecutor = &*(executor as *const HostedExecutor);
                    executor.poll();
                }
                return;
            }
        }
    }
}
