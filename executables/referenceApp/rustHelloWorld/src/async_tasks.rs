///////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Accenture
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0
//
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////////////////////////

//! Application Rust async tasks.
//!
//! **This is the file you edit to add Rust async tasks.** Define your task with
//! `#[embassy_executor::task]` and register it in [`spawn`]. Everything else
//! (the C ABI, the `critical_section` provider, the C++ host) is generic and
//! lives in [`crate::runtime_glue`] and the C++ side.

use core::fmt::Write;
use core::future::Future;
use core::pin::Pin;
use core::task::{Context, Poll};

use embassy_executor::Spawner;
use openbsw_console_out::Console;

struct YieldNow {
    yielded: bool,
}

impl YieldNow {
    fn new() -> Self {
        Self { yielded: false }
    }
}

impl Future for YieldNow {
    type Output = ();

    fn poll(mut self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<()> {
        if self.yielded {
            Poll::Ready(())
        } else {
            self.yielded = true;
            cx.waker().wake_by_ref();
            Poll::Pending
        }
    }
}

#[embassy_executor::task]
async fn demo_task_a() {
    let mut tick: u32 = 0;
    while tick < 5 {
        let _ = write!(Console, "Rust async task A: tick {}\r\n", tick);
        YieldNow::new().await;
        tick += 1;
    }
    let _ = write!(Console, "Rust async task A: done\r\n");
}

#[embassy_executor::task]
async fn demo_task_b() {
    let mut tick: u32 = 0;
    while tick < 5 {
        let _ = write!(Console, "Rust async task B: tick {}\r\n", tick);
        YieldNow::new().await;
        tick += 1;
    }
    let _ = write!(Console, "Rust async task B: done\r\n");
}

#[embassy_executor::task]
async fn background_task() {
    let mut tick: u32 = 0;
    while tick < 5 {
        let _ = write!(Console, "Rust background task: tick {}\r\n", tick);
        YieldNow::new().await;
        tick += 1;
    }
    let _ = write!(Console, "Rust background task: done\r\n");
}

#[repr(u8)]
enum Task {
    Idle = 0,
    Background = 1,
    Bsp = 2,
    Uds = 3,
    Demo = 4,
    Ethernet = 5,
    Can = 6,
    Sysadmin = 7,
    Safety = 8,
}

impl Task {
    fn from_id(id: u8) -> Option<Self> {
        Some(match id {
            0 => Self::Idle,
            1 => Self::Background,
            2 => Self::Bsp,
            3 => Self::Uds,
            4 => Self::Demo,
            5 => Self::Ethernet,
            6 => Self::Can,
            7 => Self::Sysadmin,
            8 => Self::Safety,
            _ => {
                return None;
            }
        })
    }
}

pub fn spawn(spawner: Spawner, task_id: u8) {
    let Some(task) = Task::from_id(task_id) else {
        return;
    };
    match task {
        Task::Demo => {
            spawner.spawn(demo_task_a()).ok();
            spawner.spawn(demo_task_b()).ok();
        }
        Task::Background => {
            spawner.spawn(background_task()).ok();
        }
        Task::Idle => {}
        Task::Bsp => {}
        Task::Uds => {}
        Task::Ethernet => {}
        Task::Can => {}
        Task::Sysadmin => {}
        Task::Safety => {}
    }
}
