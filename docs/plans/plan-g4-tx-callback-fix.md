---
title: Fix G4 FDCAN TX Callback Chain — Research & Plan
status: PENDING
date: 2026-03-28
---

# G4 FDCAN TX Callback Fix

## Research Findings — Correcting Earlier Analysis

After thorough code reading, the earlier lesson-learned (008) was **partially wrong**:

### What's NOT broken (contrary to earlier claim):
- `canFrameSentCallback()` is NOT commented out — it IS called from
  `FdCanTransceiver::transmitInterrupt()` line 218
- The TX callback chain IS connected: ISR → transmitInterrupt() →
  canFrameSentCallback() → async::execute() → canFrameSentAsyncCallback()
- `canFrameSentAsyncCallback()` DOES call `listener.canFrameSent()` and
  transmits the next queued frame

### What IS broken:

**1. TC interrupt is shared between listener-based and fire-and-forget TX**

S32K uses `CALLBACK_TRANSMIT_BUFFER=31` with a per-buffer interrupt mask.
Only buffer 31's TX completion triggers the callback. Demo/fire-and-forget
frames use other buffers — their completion does NOT trigger the callback.

G4 uses TC (Transmission Complete) which fires for ANY completed TX buffer.
When the demo sends 0x558 (fire-and-forget) and DoCAN sends 0x7E8 (with
listener) near-simultaneously:
- 0x558 completes → TC fires → ISR calls `canFrameSentCallback()`
- 0x7E8 completes → TC fires → ISR calls `canFrameSentCallback()` again
- But `async::execute(_canFrameSent)` drops the second enqueue (dedup)
- The 0x7E8 callback is lost → DoCAN TX stalls

S32K avoids this because demo TX completions don't trigger the callback at all.

**2. `fTxEventEnabled` is dead code**

Set in `write(frame, listener)` but never read by `transmit()`.
Originally intended for TX Event FIFO path but abandoned.

**3. Header vs implementation mismatch**

`FdCanDevice.h` line 79-80 says TX routes to line 1 via `FDCAN_ILS_SMSG`.
Code sets `ILS=0` (everything on line 0). Misleading but functionally OK
since `call_can_isr_RX()` handles both RX and TX.

## Root Cause Analysis — UPDATED after deeper research

### NOT the primary cause: async dedup
After tracing `RunnableExecutor::enqueue()`, the dedup does NOT actually
lose events. Even when `isEnqueued()` is true, `setEvent()` still fires
unconditionally (line 79). The task wakes, processes the single queued
`_canFrameSent`, which correctly pops the queue and sends the next frame.
The second `setEvent()` causes an extra wakeup with empty queue — harmless.

### NOT the primary cause: TC multiplexing
When fire-and-forget 0x558 and listener 0x7E8 complete simultaneously,
TC is an OR of all buffer completions. The ISR handles it once,
`transmitInterrupt()` checks `fTxQueue.empty()` and dispatches correctly.
One call is sufficient because the queue is sequential.

### LIKELY primary cause: CAN bus signal integrity
HIL measurements show:
- Fire-and-forget TX delivery: ~30% (Pi sees ~30% of board frames)
- Pi→board RX delivery: ~30% (board receives ~30% of Pi frames)
- UDS success: ~10% ≈ 30% × 30% (both directions must succeed)

The ~10% UDS rate is consistent with symmetric ~30% packet loss on the
physical CAN bus, NOT a software callback issue.

### Still valid: TC multiplexing is a latent bug
Even though it's not the primary cause at current loss rates, the TC
multiplexing IS a design weakness compared to S32K's per-buffer interrupt.
If the bus were healthy (99%+ delivery), the dedup issue would surface
as occasional missed callbacks. The 50ms poll would recover them.

### Still valid: fTxEventEnabled is dead code
### Still valid: header/implementation mismatch

## Fix Options

### Option A: Filter TC by buffer (match S32K pattern)
Use TX Event FIFO to identify WHICH buffer completed. Only call
`canFrameSentCallback()` if the completed buffer was the one used for
listener-based TX. Requires:
- Set EFC=1 when transmitting with listener
- Read TXEFS in `transmitISR()` to check which buffer completed
- Only call callback if the completed buffer had EFC=1

**Pro:** Clean separation like S32K. No dedup issue.
**Con:** Requires TX Event FIFO configuration + EFC bit management.

### Option B: Use a callback delegate in FdCanDevice (like S32K)
Add `fFrameSentCallback` delegate to `FdCanDevice` (like `FlexCANDevice`).
Call it from `transmitISR()` only when appropriate.

**Pro:** Matches S32K architecture exactly.
**Con:** Larger refactor of FdCanDevice.

### Option C: Make canFrameSentCallback() dedup-safe
Instead of `async::execute(_canFrameSent)`, use a counter or separate
runnables so repeated enqueues are not dropped.

**Pro:** Minimal code change.
**Con:** Works around the framework instead of fixing the root cause.

### Option D: Remove async dispatch, call canFrameSentAsyncCallback() from ISR
Skip `async::execute()` entirely for TX completion. Call the callback
directly from ISR context with proper locking.

**Pro:** Eliminates dedup entirely.
**Con:** Violates OpenBSW async model. Callback runs in ISR context.

## Recommended: Option A

Filter TC by buffer using TX Event FIFO. This matches the S32K intent
(dedicated buffer for listener TX) while using G4 hardware features.

## Implementation Steps

### Step 1: Write failing tests (TDD)

Write tests in `FdCanTransceiverTest.cpp` that prove:
1. Single write(frame, listener) → callback fires → PASS (should pass already)
2. Two writes with listener → both callbacks fire in order → PASS (should pass already)
3. Fire-and-forget TX completes between listener TX and callback → callback still fires → FAIL (this is the bug)

### Step 2: Fix FdCanDevice to track listener TX buffer

- When `fTxEventEnabled=true`, set EFC=1 on the TX buffer element
- In `transmitISR()`, check TX Event FIFO for EFC=1 entries
- Return whether the completed TX was a listener TX

### Step 3: Fix FdCanTransceiver::transmitInterrupt()

- Only call `canFrameSentCallback()` if `transmitISR()` reports a listener TX completed
- Fire-and-forget TX completions just clear the flag without triggering callback

### Step 4: Clean up dead code

- Remove or connect `fTxEventEnabled` properly
- Fix header comments to match actual ILS/ILE routing

### Step 5: HIL verification

- Flash both boards, run 50s echo test
- UDS test: 20 requests at 1s → expect >90% success (up from ~10%)

## Files to Modify

| File | Change |
|------|--------|
| `FdCanDevice.cpp` | `transmit()`: set EFC=1 when `fTxEventEnabled=true`. `transmitISR()`: return bool if listener TX completed |
| `FdCanDevice.h` | `transmitISR()` return type: `bool` instead of `void`. Fix header comments |
| `FdCanTransceiver.cpp` | `transmitInterrupt()`: only call callback if `transmitISR()` returns true |
| `FdCanTransceiverTest.cpp` | Add callback-chain tests matching S32K patterns |
