// Copyright 2024 Accenture.

#include "inputManager/AlternativeDigitalInput.h"

namespace bios
{
AlternativeDigitalInput::AlternativeDigitalInput()
: state(), edge_neg(), edge_pos(), status_old(), reser(), debounced()
{
  // workaround clang warning -Wunused-private-field
  static_cast<void>(reser);
}

/**
 * Init bios level DIGITAL
 */
void AlternativeDigitalInput::init(uint16_t /*port*/, uint16_t /*config*/, uint8_t const init_state)
{
    if (init_state != 0U)
    {
        state      = 1U;
        status_old = 1U;
        debounced  = 0xffffU;
    }
    else
    {
        state      = 0U;
        status_old = 0U;
        debounced  = 0U;
    }
    edge_neg = 0U;
    edge_pos = 0U;
}

/**
 * Process new input port state with debouncing.
 * Returns the new input state.
 */
uint8_t AlternativeDigitalInput::process(
    uint16_t const port_in, uint16_t const max_debounce, bool const inv)
{
    edge_pos = 0U;
    edge_neg = 0U;

    if (debounced > max_debounce)
    {
        debounced = max_debounce;
    }
    if (port_in != 0U)
    {
        if (debounced < max_debounce)
        {
            debounced++;
        }
        else
        {
            state = 1U;
        }
    }
    else
    {
        if (debounced != 0U)
        {
            debounced--;
        }
        else
        {
            state = 0U;
        }
    }
    state ^= static_cast<uint32_t>(inv);

    if ((state != status_old) && (state != 0U))
    {
        edge_pos = 1U;
    }
    if ((state != status_old) && (state == 0U))
    {
        edge_neg = 1U;
    }
    status_old = state;

    return static_cast<uint8_t>(state);
}

} // namespace bios
