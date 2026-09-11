///////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Accenture
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0
//
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////////////////////////

//! Reusable Embassy executor host for OpenBSW.
//!
//! This crate wraps the low-level [`embassy_executor::raw::Executor`] into a
//! [`HostedExecutor`] that a C++ Async task can drive cooperatively: each call to
//! [`HostedExecutor::poll`] does a little work and returns, so Rust async never
//! blocks the hosting C++ task. When a task wants to run again Embassy calls the
//! exported `__pender`, which forwards to the C++ `schedule_rust_runtime`, and
//! the C++ host schedules another poll.
//!
//! The wake-up is routed by *task id*: the C++ host passes the id of its hosting
//! task into [`HostedExecutor::new`], Embassy stores it as the executor context
//! and hands it straight back to `__pender`, which forwards it to
//! `schedule_rust_runtime` so the C++ side can reschedule that task.
//!
//! The [`runtime`] module builds a small fixed pool of these executors (one per
//! hosting task / priority level) for applications to wire into the C ABI.

#![no_std]

use core::marker::PhantomData;

pub mod runtime;

/// Identifies the hosting task an executor runs on.
///
/// Embassy stores an opaque `*mut ()` "context" per executor and hands it back
/// to the pender. We carry the task id through that pointer so the pender can
/// tell the C++ host which runnable to reschedule.
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub struct TaskId(u8);

impl TaskId {
    pub const fn new(id: u8) -> Self {
        Self(id)
    }

    pub const fn get(self) -> u8 {
        self.0
    }

    const fn to_context(self) -> *mut () {
        self.0 as usize as *mut ()
    }

    fn from_context(context: *mut ()) -> Self {
        let raw = context as usize;
        // A broken context would be undefined behaviour.
        assert!(raw <= (u8::MAX as usize), "task id context out of range");
        Self(raw as u8)
    }
}

/// A single Embassy executor bound to one hosting task.
///
/// Embassy ties an executor to exactly one driving task, so this type is
/// deliberately `!Send`: it must only ever be polled from the task it was
/// created on. The `PhantomData<*mut ()>` marker removes the auto `Send`/`Sync`
/// impls that the wrapped executor would otherwise provide.
///
/// [`poll`](Self::poll) drains a *snapshot* of the ready queue: tasks that wake
/// themselves *during* a poll are re-queued for the *next* poll rather than run
/// again immediately. That guarantees a single `poll()` always terminates and a
/// self-waking task (e.g. one that keeps yielding) can never starve the others.
pub struct HostedExecutor {
    inner: embassy_executor::raw::Executor,
    _not_send: PhantomData<*mut ()>,
}

impl HostedExecutor {
    /// Create an executor for the hosting task `task_id`.
    pub fn new(task_id: u8) -> Self {
        Self {
            inner: embassy_executor::raw::Executor::new(TaskId::new(task_id).to_context()),
            _not_send: PhantomData,
        }
    }

    /// Spawn the initial task(s) on this executor.
    pub fn init<F: FnOnce(embassy_executor::Spawner)>(&'static self, init: F) {
        init(self.inner.spawner());
    }

    /// Poll the executor once.
    ///
    /// # Safety
    ///
    /// Must only be called from this executor's hosting task, and never
    /// re-entrantly.
    pub unsafe fn poll(&'static self) {
        // SAFETY: guaranteed by this function's contract.
        unsafe { self.inner.poll() }
    }
}

// The C++ host entry point that reschedules the runnable for `task_id`.
#[cfg(not(test))]
unsafe extern "C" {
    fn schedule_rust_runtime(task_id: u8);
}

/// Embassy's pender: called when a task on some executor becomes ready.
#[cfg(not(test))]
#[unsafe(export_name = "__pender")]
fn __pender(context: *mut ()) {
    let task_id = TaskId::from_context(context).get();
    // SAFETY: `schedule_rust_runtime` is a plain C function supplied by the host
    // binary; passing it a `u8` is always sound.
    unsafe { schedule_rust_runtime(task_id) }
}
